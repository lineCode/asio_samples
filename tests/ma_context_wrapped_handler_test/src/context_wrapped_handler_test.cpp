//
// Copyright (c) 2018 Marat Abrarov (abrarov@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstddef>
#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <ma/config.hpp>
#include <ma/context_wrapped_handler.hpp>
#include <ma/detail/memory.hpp>
#include <ma/detail/functional.hpp>
#include <ma/detail/latch.hpp>

namespace ma {
namespace test {
namespace context_wrapped_handler {

class tracking_handler
{
private:
  typedef tracking_handler this_type;

  this_type& operator=(const this_type&);

public:
  tracking_handler(detail::latch& alloc_counter, detail::latch& dealloc_counter,
      detail::latch& invoke_counter)
    : alloc_counter_(alloc_counter)
    , dealloc_counter_(dealloc_counter)
    , invoke_counter_(invoke_counter)
  {
  }

  void operator()(detail::latch& call_counter)
  {
    call_counter.count_up();
  }

  friend void* asio_handler_allocate(std::size_t size, this_type* context)
  {
    context->alloc_counter_.count_up();
    return boost::asio::asio_handler_allocate(size);
  }

  friend void asio_handler_deallocate(void* pointer, std::size_t size,
      this_type* context)
  {
    context->dealloc_counter_.count_up();
    boost::asio::asio_handler_deallocate(pointer, size);
  }

#if defined(MA_HAS_RVALUE_REFS)

  template <typename Function>
  friend void asio_handler_invoke(MA_FWD_REF(Function) function,
      this_type* context)
  {
    context->invoke_counter_.count_up();
    boost::asio::asio_handler_invoke(detail::forward<Function>(function));
  }

#else // defined(MA_HAS_RVALUE_REFS)

  template <typename Function>
  friend void asio_handler_invoke(Function& function, this_type* context)
  {
    context->invoke_counter_.count_up();
    boost::asio::asio_handler_invoke(function);
  }

  template <typename Function>
  friend void asio_handler_invoke(const Function& function, this_type* context)
  {
    context->invoke_counter_.count_up();
    boost::asio::asio_handler_invoke(function);
  }

#endif // defined(MA_HAS_RVALUE_REFS)

private:
  detail::latch& alloc_counter_;
  detail::latch& dealloc_counter_;
  detail::latch& invoke_counter_;
}; // class tracking_handler

TEST(context_wrapped_handler, delegation)
{
  detail::latch alloc_counter;
  detail::latch dealloc_counter;
  detail::latch invoke_counter;
  detail::latch call_counter;

  boost::asio::io_service io_service;
  io_service.post(ma::make_context_wrapped_handler(
      tracking_handler(alloc_counter, dealloc_counter, invoke_counter),
      detail::bind(&detail::latch::count_up, detail::addressof(call_counter))));
  io_service.run();

  ASSERT_LE(1U, alloc_counter.value());
  ASSERT_LE(1U, dealloc_counter.value());
  ASSERT_EQ(alloc_counter.value(), dealloc_counter.value());
  ASSERT_EQ(1U, invoke_counter.value());
  ASSERT_EQ(1U, call_counter.value());
}

} // namespace context_wrapped_handler
} // namespace test
} // namespace ma
