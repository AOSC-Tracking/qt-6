// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_apis/calendar/calendar_api_url_generator.h"

#include <optional>

#include "base/strings/escape.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "google_apis/calendar/calendar_api_requests.h"
#include "google_apis/common/time_util.h"
#include "net/base/url_util.h"

namespace google_apis {

namespace calendar {

namespace {

// Hard coded URLs for communication with a google calendar server.
constexpr char kCalendarV3ColorUrl[] = "calendar/v3/colors";
constexpr char kCalendarV3EventsUrlFormat[] = "calendar/v3/calendars/%s/events";
constexpr char kCalendarV3CalendarListUrl[] =
    "calendar/v3/users/me/calendarList";
constexpr char kMaxAttendeesParameterName[] = "maxAttendees";
constexpr char kMaxResultsParameterName[] = "maxResults";
constexpr char kSingleEventsParameterName[] = "singleEvents";
constexpr char kTimeMaxParameterName[] = "timeMax";
constexpr char kTimeMinParameterName[] = "timeMin";

}  // namespace

CalendarApiUrlGenerator::CalendarApiUrlGenerator() = default;

CalendarApiUrlGenerator::CalendarApiUrlGenerator(
    const CalendarApiUrlGenerator& src) = default;

CalendarApiUrlGenerator& CalendarApiUrlGenerator::operator=(
    const CalendarApiUrlGenerator& src) = default;

CalendarApiUrlGenerator::~CalendarApiUrlGenerator() = default;

GURL CalendarApiUrlGenerator::GetCalendarEventListUrl(
    const std::string& calendar_id,
    const base::Time& start_time,
    const base::Time& end_time,
    bool single_events,
    std::optional<int> max_attendees,
    std::optional<int> max_results) const {
  GURL url;
  if (!calendar_id.empty()) {
    url = base_url_.Resolve(base::StringPrintf(
        kCalendarV3EventsUrlFormat, base::EscapePath(calendar_id).c_str()));
  } else {
    url = base_url_.Resolve(
        base::StringPrintf(kCalendarV3EventsUrlFormat,
                           base::EscapePath(kPrimaryCalendarID).c_str()));
  }
  std::string start_time_string = util::FormatTimeAsString(start_time);
  std::string end_time_string = util::FormatTimeAsString(end_time);
  url = net::AppendOrReplaceQueryParameter(url, kTimeMinParameterName,
                                           start_time_string);
  url = net::AppendOrReplaceQueryParameter(url, kTimeMaxParameterName,
                                           end_time_string);
  url = net::AppendOrReplaceQueryParameter(url, kSingleEventsParameterName,
                                           single_events ? "true" : "false");
  if (max_attendees.has_value()) {
    url = net::AppendOrReplaceQueryParameter(
        url, kMaxAttendeesParameterName,
        base::NumberToString(max_attendees.value()));
  }
  if (max_results.has_value()) {
    url = net::AppendOrReplaceQueryParameter(
        url, kMaxResultsParameterName,
        base::NumberToString(max_results.value()));
  }
  return url;
}

GURL CalendarApiUrlGenerator::GetCalendarColorListUrl() const {
  GURL url = base_url_.Resolve(kCalendarV3ColorUrl);
  return url;
}

GURL CalendarApiUrlGenerator::GetCalendarListUrl(
    std::optional<int> max_results) const {
  GURL url = base_url_.Resolve(kCalendarV3CalendarListUrl);
  if (max_results.has_value()) {
    url = net::AppendOrReplaceQueryParameter(
        url, kMaxResultsParameterName,
        base::NumberToString(max_results.value()));
  }
  return url;
}

}  // namespace calendar
}  // namespace google_apis
