#ifndef WERKZEUGKISTE_LOGGING_H
#define WERKZEUGKISTE_LOGGING_H

#if __has_include(<spdlog/spdlog.h>)
#  include <spdlog/spdlog.h>
#  include <spdlog/fmt/ostr.h>
#  define WKZ_TRACE SPDLOG_TRACE
#  define WKZ_DEBUG SPDLOG_DEBUG
#  define WKZ_INFO SPDLOG_INFO
#  define WKZ_WARN SPDLOG_WARN
#  define WKZ_ERROR SPDLOG_ERROR
#  define WKZ_CRITICAL SPDLOG_CRITICAL
#else  // has<spdlog>
#  define WKZ_TRACE(...) (void)0
#  define WKZ_DEBUG(...) (void)0
#  define WKZ_INFO(...) (void)0
#  define WKZ_WARN(...) (void)0
#  define WKZ_ERROR(...) (void)0
#  define WKZ_CRITICAL(...) (void)0
#endif  // has<spdlog>
#endif  // WERKZEUGKISTE_LOGGING_H

