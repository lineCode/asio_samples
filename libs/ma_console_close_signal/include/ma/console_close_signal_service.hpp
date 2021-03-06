//
// Copyright (c) 2010-2015 Marat Abrarov (abrarov@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef MA_CONSOLE_CLOSE_SIGNAL_SERVICE_HPP
#define MA_CONSOLE_CLOSE_SIGNAL_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio.hpp>
#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL) \
    && defined(MA_ASIO_DETAIL_SIGNAL_SET_SERVICE)
#include <boost/asio/detail/signal_set_service.hpp>
#endif

#include <ma/config.hpp>
#include <ma/windows/console_signal.hpp>
#include <ma/detail/service_base.hpp>
#include <ma/detail/utility.hpp>

#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)
#include <csignal>
#endif

namespace ma {

/// asio::io_service::service implementing console_close_signal.
class console_close_signal_service
  : public detail::service_base<console_close_signal_service>
{
private:
  typedef detail::service_base<console_close_signal_service> base_type;

#if defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)
  typedef ma::windows::console_signal_service service_impl_type;
#elif defined(MA_ASIO_DETAIL_SIGNAL_SET_SERVICE)
  typedef boost::asio::detail::signal_set_service service_impl_type;
#else
  typedef boost::asio::signal_set_service service_impl_type;
#endif

public:
  typedef service_impl_type::implementation_type implementation_type;

  explicit console_close_signal_service(boost::asio::io_service& io_service);
  void construct(implementation_type& impl);
  void destroy(implementation_type& impl);

  template <typename Handler>
  void async_wait(implementation_type& impl, MA_FWD_REF(Handler) handler);

  void cancel(implementation_type& impl, boost::system::error_code& error);

private:
  virtual void shutdown_service();

#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)
  void throw_if_error(const boost::system::error_code& error);
#endif

  service_impl_type& service_impl_;
}; // class console_close_signal_service

inline console_close_signal_service::console_close_signal_service(
    boost::asio::io_service& io_service)
  : base_type(io_service)
  , service_impl_(boost::asio::use_service<service_impl_type>(io_service))
{
}

inline void console_close_signal_service::construct(implementation_type& impl)
{
  service_impl_.construct(impl);

#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)

  boost::system::error_code error;
  service_impl_.add(impl, SIGINT, error);
  throw_if_error(error);

  service_impl_.add(impl, SIGTERM, error);
  throw_if_error(error);

#if defined(SIGQUIT)
  service_impl_.add(impl, SIGQUIT, error);
  throw_if_error(error);
#endif

#endif // !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)
}

inline void console_close_signal_service::destroy(implementation_type& impl)
{
  service_impl_.destroy(impl);
}

template <typename Handler>
void console_close_signal_service::async_wait(implementation_type& impl,
    MA_FWD_REF(Handler) handler)
{
#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL) \
    && defined(MA_ASIO_DETAIL_SIGNAL_SET_SERVICE)
  // boost::asio::detail::signal_set_service::async_wait method
  // takes handler by non-const refererence, so we need to make a temporal copy
  // before passing it to boost::asio::detail::signal_set_service::async_wait
  Handler tmp(detail::forward<Handler>(handler));
  service_impl_.async_wait(impl, tmp);
#else
  service_impl_.async_wait(impl, detail::forward<Handler>(handler));
#endif
}

inline void console_close_signal_service::cancel(
    implementation_type& impl, boost::system::error_code& error)
{
  service_impl_.cancel(impl, error);
}

inline void console_close_signal_service::shutdown_service()
{
  // Do nothing.
  // service_impl_type::shutdown_service() will be called by io_service
}

#if !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)

inline void console_close_signal_service::throw_if_error(
    const boost::system::error_code& error)
{
  if (error)
  {
    boost::throw_exception(boost::system::system_error(error));
  }
}

#endif // !defined(MA_HAS_WINDOWS_CONSOLE_SIGNAL)

} // namespace ma

#endif // MA_CONSOLE_CLOSE_SIGNAL_SERVICE_HPP
