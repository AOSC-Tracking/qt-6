// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/data_model/credit_card_benefit.h"

#include "components/autofill/core/browser/data_model/credit_card_benefit_test_api.h"
#include "components/autofill/core/common/autofill_clock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace autofill {

namespace {

const CreditCardBenefit::BenefitId kArbitraryBenefitId =
    CreditCardBenefit::BenefitId("id");
const CreditCardBenefit::LinkedCardInstrumentId kArbitraryInstrumentId =
    CreditCardBenefit::LinkedCardInstrumentId(1234);
const std::u16string kArbitraryDescription = u"description";
const base::Time kArbitraryPastTime = AutofillClock::Now() - base::Days(10);
const base::Time kArbitraryFutureTime = AutofillClock::Now() + base::Days(10);
const CreditCardCategoryBenefit::BenefitCategory kArbitraryBenefitCategory =
    CreditCardCategoryBenefit::BenefitCategory::kDining;

}  // namespace

// Test equals when flat rate benefits are different.
TEST(CreditCardBenefitTest, CompareFlatRateBenefits) {
  CreditCardFlatRateBenefit benefit = CreditCardFlatRateBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryPastTime, kArbitraryFutureTime);
  CreditCardFlatRateBenefit other_benefit = CreditCardFlatRateBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryPastTime, kArbitraryFutureTime);

  // Same benefit.
  EXPECT_TRUE(benefit == other_benefit);

  // Different IDs.
  test_api(other_benefit)
      .SetBenefitIdForTesting(CreditCardBenefit::BenefitId("id2"));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetBenefitIdForTesting(benefit.benefit_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different instrument IDs.
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(
          CreditCardBenefit::LinkedCardInstrumentId(2234));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(benefit.linked_card_instrument_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit descriptions.
  test_api(other_benefit).SetBenefitDescriptionForTesting(u"description2");
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetBenefitDescriptionForTesting(benefit.benefit_description());
  EXPECT_TRUE(benefit == other_benefit);

  // Different start times.
  test_api(other_benefit).SetStartTimeForTesting(base::Time::Min());
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetStartTimeForTesting(benefit.start_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different end times.
  test_api(other_benefit)
      .SetEndTimeForTesting(AutofillClock::Now() + base::Days(1));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetEndTimeForTesting(benefit.expiry_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit types.
  CreditCardCategoryBenefit category_benefit = CreditCardCategoryBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryBenefitCategory,
      kArbitraryDescription, kArbitraryPastTime, kArbitraryFutureTime);
  EXPECT_TRUE(benefit != category_benefit);
}

// Test equals when category benefits are different.
TEST(CreditCardBenefitTest, CompareCategoryBenefits) {
  CreditCardCategoryBenefit benefit = CreditCardCategoryBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryBenefitCategory,
      kArbitraryDescription, kArbitraryPastTime, kArbitraryFutureTime);
  CreditCardCategoryBenefit other_benefit = CreditCardCategoryBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryBenefitCategory,
      kArbitraryDescription, kArbitraryPastTime, kArbitraryFutureTime);

  // Same benefit.
  EXPECT_TRUE(benefit == other_benefit);

  // Different IDs.
  test_api(other_benefit)
      .SetBenefitIdForTesting(CreditCardBenefit::BenefitId("id2"));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetBenefitIdForTesting(benefit.benefit_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different instrument IDs.
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(
          CreditCardBenefit::LinkedCardInstrumentId(2234));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(benefit.linked_card_instrument_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit categories.
  test_api(other_benefit)
      .SetBenefitCategoryForTesting(
          CreditCardCategoryBenefit::BenefitCategory::kFlights);
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetBenefitCategoryForTesting(benefit.benefit_category());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit descriptions.
  test_api(other_benefit).SetBenefitDescriptionForTesting(u"description2");
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetBenefitDescriptionForTesting(benefit.benefit_description());
  EXPECT_TRUE(benefit == other_benefit);

  // Different start times.
  test_api(other_benefit).SetStartTimeForTesting(base::Time::Min());
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetStartTimeForTesting(benefit.start_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different end times.
  test_api(other_benefit)
      .SetEndTimeForTesting(AutofillClock::Now() + base::Days(1));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetEndTimeForTesting(benefit.expiry_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit types.
  CreditCardFlatRateBenefit flat_rate_benefit = CreditCardFlatRateBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryPastTime, kArbitraryFutureTime);
  EXPECT_TRUE(benefit != flat_rate_benefit);
}

// Test equals when merchant benefits are different.
TEST(CreditCardBenefitTest, CompareMerchantBenefits) {
  base::flat_set<url::Origin> kArbitraryDomains = {
      url::Origin::Create(GURL("http://www.example.com"))};
  CreditCardMerchantBenefit benefit = CreditCardMerchantBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryDomains, kArbitraryPastTime, kArbitraryFutureTime);
  CreditCardMerchantBenefit other_benefit = CreditCardMerchantBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryDomains, kArbitraryPastTime, kArbitraryFutureTime);

  // Same benefit.
  EXPECT_TRUE(benefit == other_benefit);

  // Different IDs.
  test_api(other_benefit)
      .SetBenefitIdForTesting(CreditCardBenefit::BenefitId("id2"));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetBenefitIdForTesting(benefit.benefit_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different instrument IDs.
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(
          CreditCardBenefit::LinkedCardInstrumentId(2234));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetLinkedCardInstrumentIdForTesting(benefit.linked_card_instrument_id());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit descriptions.
  test_api(other_benefit).SetBenefitDescriptionForTesting(u"description2");
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetBenefitDescriptionForTesting(benefit.benefit_description());
  EXPECT_TRUE(benefit == other_benefit);

  // Different merchant domains.
  test_api(other_benefit)
      .SetMerchantDomainsForTesting(
          {url::Origin::Create(GURL("http://www.example2.com"))});
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit)
      .SetMerchantDomainsForTesting(benefit.merchant_domains());
  EXPECT_TRUE(benefit == other_benefit);

  // Different start times.
  test_api(other_benefit).SetStartTimeForTesting(base::Time::Min());
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetStartTimeForTesting(benefit.start_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different end times.
  test_api(other_benefit)
      .SetEndTimeForTesting(AutofillClock::Now() + base::Days(1));
  EXPECT_TRUE(benefit != other_benefit);
  test_api(other_benefit).SetEndTimeForTesting(benefit.expiry_time());
  EXPECT_TRUE(benefit == other_benefit);

  // Different benefit types.
  CreditCardFlatRateBenefit flat_rate_benefit = CreditCardFlatRateBenefit(
      kArbitraryBenefitId, kArbitraryInstrumentId, kArbitraryDescription,
      kArbitraryPastTime, kArbitraryFutureTime);
  EXPECT_TRUE(benefit != flat_rate_benefit);
}

// Test that `IsValid` returns true for valid benefits.
TEST(CreditCardBenefitTest, BenefitValidation_ValidBenefits) {
  EXPECT_TRUE(
      CreditCardFlatRateBenefit(kArbitraryBenefitId, kArbitraryInstrumentId,
                                kArbitraryDescription, kArbitraryPastTime,
                                kArbitraryFutureTime)
          .IsValid());

  EXPECT_TRUE(CreditCardCategoryBenefit(
                  kArbitraryBenefitId, kArbitraryInstrumentId,
                  kArbitraryBenefitCategory, kArbitraryDescription,
                  kArbitraryPastTime, kArbitraryFutureTime)
                  .IsValid());

  EXPECT_TRUE(CreditCardMerchantBenefit(
                  kArbitraryBenefitId, kArbitraryInstrumentId,
                  kArbitraryDescription,
                  {{url::Origin::Create(GURL("http://www.example.com"))}},
                  kArbitraryPastTime, kArbitraryFutureTime)
                  .IsValid());
}

// Test that `IsValid` returns false for benefits without IDs.
TEST(CreditCardBenefitTest, BenefitValidation_EmptyBenefitId) {
  CreditCardBenefit::BenefitId empty_id = CreditCardBenefit::BenefitId();

  EXPECT_FALSE(CreditCardFlatRateBenefit(
                   empty_id, kArbitraryInstrumentId, kArbitraryDescription,
                   kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());

  EXPECT_FALSE(CreditCardCategoryBenefit(
                   empty_id, kArbitraryInstrumentId, kArbitraryBenefitCategory,
                   kArbitraryDescription, kArbitraryPastTime,
                   kArbitraryFutureTime)
                   .IsValid());

  EXPECT_FALSE(CreditCardMerchantBenefit(
                   empty_id, kArbitraryInstrumentId, kArbitraryDescription,
                   {{url::Origin::Create(GURL("http://www.example.com"))}},
                   kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());
}

// Test that `IsValid` returns false for benefits without instrument ID.
TEST(CreditCardBenefitTest, BenefitValidation_EmptyInstrumentId) {
  CreditCardBenefit::LinkedCardInstrumentId empty_instrument_id =
      CreditCardBenefit::LinkedCardInstrumentId();

  EXPECT_FALSE(
      CreditCardFlatRateBenefit(kArbitraryBenefitId, empty_instrument_id,
                                kArbitraryDescription, kArbitraryPastTime,
                                kArbitraryFutureTime)
          .IsValid());

  EXPECT_FALSE(CreditCardCategoryBenefit(
                   kArbitraryBenefitId, empty_instrument_id,
                   kArbitraryBenefitCategory, kArbitraryDescription,
                   kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());

  EXPECT_FALSE(CreditCardMerchantBenefit(
                   kArbitraryBenefitId, empty_instrument_id,
                   kArbitraryDescription,
                   {{url::Origin::Create(GURL("http://www.example.com"))}},
                   kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());
}

// Test that `IsValid` returns false for benefits with empty description.
TEST(CreditCardBenefitTest, BenefitValidation_EmptyDescriptions) {
  std::u16string empty_description = u"";

  EXPECT_FALSE(CreditCardFlatRateBenefit(
                   kArbitraryBenefitId, kArbitraryInstrumentId,
                   empty_description, kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());

  EXPECT_FALSE(
      CreditCardCategoryBenefit(kArbitraryBenefitId, kArbitraryInstrumentId,
                                kArbitraryBenefitCategory, empty_description,
                                kArbitraryPastTime, kArbitraryFutureTime)
          .IsValid());

  EXPECT_FALSE(CreditCardMerchantBenefit(
                   kArbitraryBenefitId, kArbitraryInstrumentId,
                   empty_description,
                   {{url::Origin::Create(GURL("http://www.example.com"))}},
                   kArbitraryPastTime, kArbitraryFutureTime)
                   .IsValid());
}

// Test that `IsValid` returns false for expired benefits.
TEST(CreditCardBenefitTest, BenefitValidation_InvalidEndDates) {
  base::Time expired_time = AutofillClock::Now() - base::Days(5);

  EXPECT_FALSE(CreditCardFlatRateBenefit(
                   kArbitraryBenefitId, kArbitraryInstrumentId,
                   kArbitraryDescription, kArbitraryPastTime, expired_time)
                   .IsValid());

  EXPECT_FALSE(CreditCardCategoryBenefit(
                   kArbitraryBenefitId, kArbitraryInstrumentId,
                   kArbitraryBenefitCategory, kArbitraryDescription,
                   kArbitraryPastTime, expired_time)
                   .IsValid());

  EXPECT_FALSE(CreditCardMerchantBenefit(
                   kArbitraryBenefitId, kArbitraryInstrumentId,
                   kArbitraryDescription,
                   {{url::Origin::Create(GURL("http://www.example.com"))}},
                   kArbitraryPastTime, expired_time)
                   .IsValid());
}

// Test that `IsValid` returns false for category benefit with unknown category.
TEST(CreditCardBenefitTest, BenefitValidation_UnknownCategory) {
  EXPECT_FALSE(
      CreditCardCategoryBenefit(
          kArbitraryBenefitId, kArbitraryInstrumentId,
          CreditCardCategoryBenefit::BenefitCategory::kUnknownBenefitCategory,
          kArbitraryDescription, kArbitraryPastTime, kArbitraryFutureTime)
          .IsValid());
}

// Test that `IsValid` returns false for merchant benefit with empty
// merchant domain list.
TEST(CreditCardBenefitTest, BenefitValidation_EmptyDomainList) {
  EXPECT_FALSE(
      CreditCardMerchantBenefit(kArbitraryBenefitId, kArbitraryInstrumentId,
                                kArbitraryDescription, {}, kArbitraryPastTime,
                                kArbitraryFutureTime)
          .IsValid());
}

}  // namespace autofill
