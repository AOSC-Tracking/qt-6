// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/data_model/autofill_i18n_api.h"

#include <string>
#include <type_traits>

#include "base/containers/contains.h"
#include "base/ranges/algorithm.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/scoped_feature_list.h"
#include "components/autofill/core/browser/data_model/autofill_i18n_formatting_expressions.h"
#include "components/autofill/core/browser/data_model/autofill_i18n_parsing_expressions.h"
#include "components/autofill/core/browser/data_model/autofill_structured_address.h"
#include "components/autofill/core/browser/data_model/autofill_structured_address_component_test_api.h"
#include "components/autofill/core/browser/data_model/autofill_structured_address_format_provider.h"
#include "components/autofill/core/browser/data_model/autofill_structured_address_name.h"
#include "components/autofill/core/browser/field_types.h"
#include "components/autofill/core/browser/geo/country_data.h"
#include "components/autofill/core/common/autofill_features.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace autofill::i18n_model_definition {

namespace {

// Checks that the AddressComponent graph has no cycles.
bool IsTree(AddressComponent* node, FieldTypeSet* visited_types) {
  if (visited_types->contains(node->GetStorageType()) ||
      visited_types->contains_any(node->GetAdditionalSupportedFieldTypes())) {
    // Repeated types exist in the tree.
    return false;
  }
  visited_types->insert(node->GetStorageType());
  visited_types->insert_all(node->GetAdditionalSupportedFieldTypes());
  if (node->Subcomponents().empty()) {
    return true;
  }
  return base::ranges::all_of(node->Subcomponents(),
                              [&visited_types](AddressComponent* child) {
                                return IsTree(child, visited_types);
                              });
}
}  // namespace

class AutofillI18nApiTest : public testing::Test {
 public:
  AutofillI18nApiTest() {
    feature_list_.InitWithFeatures(
        {
            features::kAutofillUseI18nAddressModel,
            features::kAutofillUseDEAddressModel,
        },
        {});
  }
  ~AutofillI18nApiTest() override = default;

 private:
  base::test::ScopedFeatureList feature_list_;
};

TEST_F(AutofillI18nApiTest, GetAddressComponentModel_ReturnsNonEmptyModel) {
  for (const auto& [country_code, properties] : kAutofillModelRules) {
      // Make sure that the process of building the model finishes and returns a
      // non empty hierarchy.
      AddressComponentsStore model = CreateAddressComponentModel(
          AddressCountryCode(std::string(country_code)));

      FieldTypeSet field_type_set;
      model.Root()->GetSupportedTypes(&field_type_set);
      EXPECT_FALSE(field_type_set.empty());
      EXPECT_FALSE(field_type_set.contains_any(
          {NO_SERVER_DATA, UNKNOWN_TYPE, EMPTY_TYPE}));

      EXPECT_EQ(test_api(model.Root()).GetRootNode().GetStorageType(),
                ADDRESS_HOME_ADDRESS);
    }
}

TEST_F(AutofillI18nApiTest, GetAddressComponentModel_ReturnedModelIsTree) {
  for (const auto& [country_code, tree_def] : kAutofillModelRules) {
    // Currently, the model for kAddressModel should comprise all the nodes in
    // the rules.
    AddressComponentsStore model = CreateAddressComponentModel(
        AddressCountryCode(std::string(country_code)));
    AddressComponent* root = model.Root();

    FieldTypeSet supported_types;
    EXPECT_TRUE(IsTree(root, &supported_types));

    // Test that all field types in the country rules are accessible through
    // the root (i.e. the tree is connected).
    for (const auto& [node_type, children_types] : tree_def) {
      EXPECT_TRUE(test_api(root).GetNodeForType(node_type));

      for (FieldType child_type : children_types) {
        EXPECT_TRUE(test_api(root).GetNodeForType(child_type));
      }
    }
  }
}

TEST_F(AutofillI18nApiTest, GetAddressComponentModel_CountryNodeHasValue) {
  for (const auto& [country_code, tree_def] : kAutofillModelRules) {
    AddressComponentsStore model = CreateAddressComponentModel(
        AddressCountryCode(std::string(country_code)));
    std::u16string expected_country =
        country_code != kLegacyHierarchyCountryCodeString
            ? base::UTF8ToUTF16(country_code)
            : u"";
    EXPECT_EQ(model.Root()->GetValueForType(ADDRESS_HOME_COUNTRY),
              expected_country);
  }
}

TEST_F(AutofillI18nApiTest, GetLegacyAddressHierarchy) {
  // "Countries that have not been migrated to the new Autofill i18n model
  // should use the legacy hierarchy (stored in a dummy country XX)."

  // Set up expected legacy hierarchy for non-migrated country.
  ASSERT_FALSE(kAutofillModelRules.contains("ES"));
  AddressComponentsStore legacy_address_hierarchy_es =
      CreateAddressComponentModel(AddressCountryCode("ES"));

  AddressComponentsStore legacy_address_hierarchy_xx =
      CreateAddressComponentModel(kLegacyHierarchyCountryCode);
  legacy_address_hierarchy_xx.Root()->SetValueForType(
      ADDRESS_HOME_COUNTRY, u"ES", VerificationStatus::kObserved);
  EXPECT_TRUE(legacy_address_hierarchy_xx.Root()->SameAs(
      *legacy_address_hierarchy_es.Root()));
}

TEST_F(AutofillI18nApiTest, GetFormattingExpressions) {
  auto* format_pattern_provider =
      StructuredAddressesFormatProvider::GetInstance();

  // Check addresses with supported i18n hierarchies. The expected value is
  // contained in `kAutofillFormattingRulesMap`.
  EXPECT_EQ(
      GetFormattingExpression(ADDRESS_HOME_OVERFLOW, AddressCountryCode("MX")),
      u"${ADDRESS_HOME_BETWEEN_STREETS_OR_LANDMARK;;}");
  EXPECT_EQ(GetFormattingExpression(ADDRESS_HOME_OVERFLOW_AND_LANDMARK,
                                    AddressCountryCode("BR")),
            u"${ADDRESS_HOME_OVERFLOW;;}\n${ADDRESS_HOME_LANDMARK;Ponto de "
            u"referência: ;}");

  // If the country has no custom entry, it should default to the legacy
  // expressions.
  EXPECT_EQ(GetFormattingExpression(ADDRESS_HOME_SUBPREMISE,
                                    AddressCountryCode("ES")),
            u"${ADDRESS_HOME_APT_NUM;Apt. ;}, ${ADDRESS_HOME_FLOOR;Floor ;}");

  // If the type does not refer to an address, use the legacy code instead.
  EXPECT_EQ(GetFormattingExpression(NAME_FULL, AddressCountryCode("ES")),
            format_pattern_provider->GetPattern(NAME_FULL, "ES"));
}

TEST_F(AutofillI18nApiTest, ParseValueByI18nRegularExpression) {
  std::string apt_str = "sala 10";
  auto* it = kAutofillParsingRulesMap.find({"BR", ADDRESS_HOME_APT});

  ASSERT_TRUE(it != kAutofillParsingRulesMap.end());
  EXPECT_EQ(ParseValueByI18nRegularExpression(apt_str, ADDRESS_HOME_APT,
                                              AddressCountryCode("BR")),
            it->second->Parse(apt_str));

  std::string street_address = "street no 123 apt 10";

  // Parsing expression for address street in not available for Spain.
  ASSERT_TRUE(
      kAutofillParsingRulesMap.find({"ES", ADDRESS_HOME_STREET_ADDRESS}) ==
      kAutofillParsingRulesMap.end());
  // In that case the legacy expression is used (if available).
  EXPECT_EQ(ParseValueByI18nRegularExpression(street_address,
                                              ADDRESS_HOME_STREET_ADDRESS,
                                              AddressCountryCode("ES")),
            ParseValueByI18nRegularExpression(street_address,
                                              ADDRESS_HOME_STREET_ADDRESS,
                                              kLegacyHierarchyCountryCode));
}

TEST_F(AutofillI18nApiTest, GetStopwordsExpression) {
  // The expected values are contained in `kAutofillModelStopwords`.
  EXPECT_EQ(u"Ponto de referência:",
            GetStopwordsExpression(ADDRESS_HOME_OVERFLOW_AND_LANDMARK,
                                   AddressCountryCode("BR")));
  EXPECT_EQ(u"Andar", GetStopwordsExpression(ADDRESS_HOME_SUBPREMISE,
                                             AddressCountryCode("BR")));
  EXPECT_EQ(u"Entre Calles",
            GetStopwordsExpression(ADDRESS_HOME_BETWEEN_STREETS_OR_LANDMARK,
                                   AddressCountryCode("MX")));
  EXPECT_EQ(u"Apt\\.|Floor", GetStopwordsExpression(ADDRESS_HOME_SUBPREMISE,
                                                    AddressCountryCode("XX")));
  EXPECT_EQ(std::nullopt, GetStopwordsExpression(ADDRESS_HOME_OVERFLOW,
                                                 AddressCountryCode("MX")));
  EXPECT_EQ(std::nullopt,
            GetStopwordsExpression(ADDRESS_HOME_BETWEEN_STREETS_OR_LANDMARK,
                                   AddressCountryCode("")));
}

TEST_F(AutofillI18nApiTest, IsTypeEnabledForCountry) {
  CountryDataMap* country_data_map = CountryDataMap::GetInstance();
  for (const std::string& country_code : country_data_map->country_codes()) {
    AddressCountryCode address_country_code{country_code};
    AddressComponentsStore store =
        CreateAddressComponentModel(address_country_code);

    for (std::underlying_type_t<FieldType> i = 0; i < MAX_VALID_FIELD_TYPE;
         ++i) {
      FieldType field_type = ToSafeFieldType(i, NO_SERVER_DATA);
      if (field_type == NO_SERVER_DATA) {
        continue;
      }
      SCOPED_TRACE(testing::Message()
                   << "Testing type " << FieldTypeToStringView(field_type)
                   << " in country " << address_country_code);

      bool is_contained =
          test_api(store.Root()).GetNodeForType(field_type) != nullptr;
      EXPECT_EQ(is_contained,
                IsTypeEnabledForCountry(field_type, address_country_code));
    }
  }
}
}  // namespace autofill::i18n_model_definition
