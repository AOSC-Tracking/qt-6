// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/form_parsing/regex_patterns.h"

#include "base/check.h"
#include "base/notreached.h"
#include "components/autofill/core/browser/form_parsing/regex_patterns_inl.h"
#include "components/autofill/core/browser/heuristic_source.h"
#include "components/autofill/core/common/autofill_features.h"

namespace autofill {

namespace {

// Returns the span of MatchPatternRefs for the given pattern name, language
// code, and pattern source.
//
// Hits a CHECK if the given pattern source contains no patterns for the given
// name.
//
// Falls back to the union of all patterns of a the given name in the given
// pattern source if there are no patterns for the given language.
base::span<const MatchPatternRef> GetMatchPatterns(
    std::string_view name,
    std::string_view language_code,
    PatternSource pattern_source) {
  auto* it = kPatternMap.find(std::make_pair(name, language_code));
  if (!language_code.empty() && it == kPatternMap.end())
    it = kPatternMap.find(std::make_pair(name, ""));
  CHECK(it != kPatternMap.end());
#if BUILDFLAG(USE_INTERNAL_AUTOFILL_PATTERNS)
  switch (pattern_source) {
    case PatternSource::kDefault:
      return it->second[0];
    case PatternSource::kExperimental:
      return it->second[1];
    case PatternSource::kNextGen:
      return it->second[2];
    case PatternSource::kLegacy:
      return it->second[3];
  }
#else
  switch (pattern_source) {
    case PatternSource::kLegacy:
      return it->second[0];
  }
#endif
  NOTREACHED();
  return {};
}

}  // namespace

std::optional<PatternSource> GetActivePatternSource() {
  return HeuristicSourceToPatternSource(GetActiveHeuristicSource());
}

base::span<const MatchPatternRef> GetMatchPatterns(
    std::string_view name,
    std::optional<LanguageCode> language_code,
    PatternSource pattern_source) {
  return language_code ? GetMatchPatterns(name, **language_code, pattern_source)
                       : GetMatchPatterns(name, "", pattern_source);
}

base::span<const MatchPatternRef> GetMatchPatterns(
    FieldType type,
    std::optional<LanguageCode> language_code,
    PatternSource pattern_source) {
  return GetMatchPatterns(FieldTypeToStringView(type), language_code,
                          pattern_source);
}

bool IsSupportedLanguageCode(LanguageCode language_code) {
  return kLanguages.contains(*language_code);
}

// The dereferencing operator implements the distinction between ordinary and
// supplementary patterns (see the header). This is why it must return a copy,
// rather than a const reference.
MatchingPattern MatchPatternRef::operator*() const {
  const MatchingPattern& p = kPatterns[index()];
  return {
      .positive_pattern = p.positive_pattern,
      .negative_pattern = p.negative_pattern,
      .positive_score = p.positive_score,
      .match_field_attributes =
          is_supplementary() ? DenseSet<MatchAttribute>{MatchAttribute::kName}
                             : p.match_field_attributes,
      .form_control_types = p.form_control_types,
  };
}

}  // namespace autofill
