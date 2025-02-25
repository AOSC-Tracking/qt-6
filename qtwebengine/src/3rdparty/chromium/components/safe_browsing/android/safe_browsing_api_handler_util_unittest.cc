// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/safe_browsing/android/safe_browsing_api_handler_util.h"

#include <string>

#include "base/strings/stringprintf.h"
#include "components/safe_browsing/core/browser/db/metadata.pb.h"
#include "components/safe_browsing/core/browser/db/util.h"
#include "components/safe_browsing/core/browser/db/v4_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace safe_browsing {

class SafeBrowsingApiHandlerUtilTest : public ::testing::Test {
 protected:
  SBThreatType threat_;
  ThreatMetadata meta_;
  const ThreatMetadata empty_meta_;

  UmaRemoteCallResult ResetAndParseJson(const std::string& json) {
    threat_ = SB_THREAT_TYPE_EXTENSION;  // Should never be seen
    meta_ = ThreatMetadata();
    return ParseJsonFromGMSCore(json, &threat_, &meta_);
  }
};

TEST_F(SafeBrowsingApiHandlerUtilTest, BadJson) {
  EXPECT_EQ(UmaRemoteCallResult::JSON_EMPTY, ResetAndParseJson(""));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_FAILED_TO_PARSE, ResetAndParseJson("{"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_FAILED_TO_PARSE, ResetAndParseJson("[]"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_FAILED_TO_PARSE,
            ResetAndParseJson("{\"matches\":\"foo\"}"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_UNKNOWN_THREAT,
            ResetAndParseJson("{\"matches\":[{}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_UNKNOWN_THREAT,
            ResetAndParseJson("{\"matches\":[{\"threat_type\":\"junk\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::JSON_UNKNOWN_THREAT,
            ResetAndParseJson("{\"matches\":[{\"threat_type\":\"999\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_SAFE, threat_);
  EXPECT_EQ(empty_meta_, meta_);
}

TEST_F(SafeBrowsingApiHandlerUtilTest, BasicThreats) {
  EXPECT_EQ(UmaRemoteCallResult::MATCH,
            ResetAndParseJson("{\"matches\":[{\"threat_type\":\"4\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_URL_MALWARE, threat_);
  EXPECT_EQ(empty_meta_, meta_);

  EXPECT_EQ(UmaRemoteCallResult::MATCH,
            ResetAndParseJson("{\"matches\":[{\"threat_type\":\"5\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_URL_PHISHING, threat_);
  EXPECT_EQ(empty_meta_, meta_);
}

TEST_F(SafeBrowsingApiHandlerUtilTest, MultipleThreats) {
  EXPECT_EQ(
      UmaRemoteCallResult::MATCH,
      ResetAndParseJson(
          "{\"matches\":[{\"threat_type\":\"4\"}, {\"threat_type\":\"5\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_URL_MALWARE, threat_);
  EXPECT_EQ(empty_meta_, meta_);
}

TEST_F(SafeBrowsingApiHandlerUtilTest, PopulationId) {
  ThreatMetadata expected;

  EXPECT_EQ(UmaRemoteCallResult::MATCH,
            ResetAndParseJson("{\"matches\":[{\"threat_type\":\"4\", "
                              "\"UserPopulation\":\"foobarbazz\"}]}"));
  EXPECT_EQ(SB_THREAT_TYPE_URL_MALWARE, threat_);
  expected.population_id = "foobarbazz";
  EXPECT_EQ(expected, meta_);
  // Test the ThreatMetadata comparator for this field.
  EXPECT_NE(empty_meta_, meta_);
}

TEST_F(SafeBrowsingApiHandlerUtilTest, SubresourceFilterSubTypes) {
  typedef SubresourceFilterLevel Level;
  typedef SubresourceFilterType Type;
  const struct {
    const char* abusive_type;
    const char* bas_type;
    SubresourceFilterMatch expected_match;
  } test_cases[] = {
      {"warn",
       "enforce",
       {{Type::ABUSIVE, Level::WARN}, {Type::BETTER_ADS, Level::ENFORCE}}},
      {nullptr, "warn", {{Type::BETTER_ADS, Level::WARN}}},
      {"asdf",
       "",
       {{Type::ABUSIVE, Level::ENFORCE}, {Type::BETTER_ADS, Level::ENFORCE}}},
      {"warn", nullptr, {{Type::ABUSIVE, Level::WARN}}},
      {nullptr, nullptr, {}},
      {"",
       "",
       {{Type::ABUSIVE, Level::ENFORCE}, {Type::BETTER_ADS, Level::ENFORCE}}},
  };

  for (const auto& test_case : test_cases) {
    static constexpr char kJson[] = R"({
        "matches" : [{
          "threat_type":"13"
          %s
          %s
        }]
      })";
    auto put_kv = [](const char* k, const char* v) {
      if (!v) {
        return std::string();
      }
      return base::StringPrintf(",\"%s\":\"%s\"", k, v);
    };
    std::string json = base::StringPrintf(
        kJson, put_kv("sf_absv", test_case.abusive_type).c_str(),
        put_kv("sf_bas", test_case.bas_type).c_str());
    SCOPED_TRACE(testing::Message() << json);

    ASSERT_EQ(UmaRemoteCallResult::MATCH, ResetAndParseJson(json));
    EXPECT_EQ(SB_THREAT_TYPE_SUBRESOURCE_FILTER, threat_);

    ThreatMetadata expected;
    expected.subresource_filter_match = test_case.expected_match;
    EXPECT_EQ(expected, meta_);
  }
}

TEST_F(SafeBrowsingApiHandlerUtilTest, GetThreatMetadataFromSafeBrowsingApi) {
  typedef SubresourceFilterLevel Level;
  typedef SubresourceFilterType Type;
  const struct {
    SafeBrowsingJavaThreatType threat_type;
    std::vector<int> threat_attributes;
    SubresourceFilterMatch expected_match;
  } test_cases[] = {
      {SafeBrowsingJavaThreatType::SOCIAL_ENGINEERING, {}, {}},
      {SafeBrowsingJavaThreatType::ABUSIVE_EXPERIENCE_VIOLATION,
       {},
       {{Type::ABUSIVE, Level::ENFORCE}}},
      {SafeBrowsingJavaThreatType::ABUSIVE_EXPERIENCE_VIOLATION,
       {static_cast<int>(SafeBrowsingJavaThreatAttribute::CANARY)},
       {{Type::ABUSIVE, Level::WARN}}},
      {SafeBrowsingJavaThreatType::ABUSIVE_EXPERIENCE_VIOLATION,
       {static_cast<int>(SafeBrowsingJavaThreatAttribute::FRAME_ONLY)},
       {{Type::ABUSIVE, Level::ENFORCE}}},
      {SafeBrowsingJavaThreatType::BETTER_ADS_VIOLATION,
       {},
       {{Type::BETTER_ADS, Level::ENFORCE}}},
      {SafeBrowsingJavaThreatType::BETTER_ADS_VIOLATION,
       {static_cast<int>(SafeBrowsingJavaThreatAttribute::CANARY)},
       {{Type::BETTER_ADS, Level::WARN}}},
      {SafeBrowsingJavaThreatType::BETTER_ADS_VIOLATION,
       {static_cast<int>(SafeBrowsingJavaThreatAttribute::FRAME_ONLY)},
       {{Type::BETTER_ADS, Level::ENFORCE}}}};

  for (const auto& test_case : test_cases) {
    ThreatMetadata metadata = GetThreatMetadataFromSafeBrowsingApi(
        test_case.threat_type, test_case.threat_attributes);
    ThreatMetadata expected;
    expected.subresource_filter_match = test_case.expected_match;
    EXPECT_EQ(expected, metadata);
  }
}

}  // namespace safe_browsing
