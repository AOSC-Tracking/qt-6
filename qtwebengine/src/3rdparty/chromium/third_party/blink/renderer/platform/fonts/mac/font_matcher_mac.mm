/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Nicholas Shanks <webkit@nickshanks.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "third_party/blink/renderer/platform/fonts/mac/font_matcher_mac.h"

#import <AppKit/AppKit.h>
#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>
#import <math.h>

#include "base/apple/bridging.h"
#include "base/apple/foundation_util.h"
#include "base/apple/scoped_cftyperef.h"
#include "third_party/blink/renderer/platform/fonts/font_cache.h"
#include "third_party/blink/renderer/platform/fonts/font_selection_types.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"
#import "third_party/blink/renderer/platform/wtf/hash_set.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#import "third_party/blink/renderer/platform/wtf/text/atomic_string_hash.h"
#import "third_party/blink/renderer/platform/wtf/text/string_impl.h"

using base::apple::CFCast;
using base::apple::GetValueFromDictionary;
using base::apple::ScopedCFTypeRef;

namespace blink {

namespace {

const FourCharCode kWeightTag = 'wght';
const FourCharCode kWidthTag = 'wdth';

const int kCTNormalTraitsValue = 0;

const NSFontTraitMask SYNTHESIZED_FONT_TRAITS =
    (NSBoldFontMask | NSItalicFontMask);

const NSFontTraitMask IMPORTANT_FONT_TRAITS =
    (NSCompressedFontMask | NSCondensedFontMask | NSExpandedFontMask |
     NSItalicFontMask | NSNarrowFontMask | NSPosterFontMask |
     NSSmallCapsFontMask);

BOOL AcceptableChoice(NSFontTraitMask desired_traits,
                      NSFontTraitMask candidate_traits) {
  desired_traits &= ~SYNTHESIZED_FONT_TRAITS;
  return (candidate_traits & desired_traits) == desired_traits;
}

BOOL BetterChoice(NSFontTraitMask desired_traits,
                  NSInteger desired_weight,
                  NSFontTraitMask chosen_traits,
                  NSInteger chosen_weight,
                  NSFontTraitMask candidate_traits,
                  NSInteger candidate_weight) {
  if (!AcceptableChoice(desired_traits, candidate_traits))
    return NO;

  // A list of the traits we care about.
  // The top item in the list is the worst trait to mismatch; if a font has this
  // and we didn't ask for it, we'd prefer any other font in the family.
  const NSFontTraitMask kMasks[] = {NSPosterFontMask,    NSSmallCapsFontMask,
                                    NSItalicFontMask,    NSCompressedFontMask,
                                    NSCondensedFontMask, NSExpandedFontMask,
                                    NSNarrowFontMask};

  for (NSFontTraitMask mask : kMasks) {
    BOOL desired = (desired_traits & mask) != 0;
    BOOL chosen_has_unwanted_trait = desired != ((chosen_traits & mask) != 0);
    BOOL candidate_has_unwanted_trait =
        desired != ((candidate_traits & mask) != 0);
    if (!candidate_has_unwanted_trait && chosen_has_unwanted_trait)
      return YES;
    if (!chosen_has_unwanted_trait && candidate_has_unwanted_trait)
      return NO;
  }

  NSInteger chosen_weight_delta_magnitude = abs(chosen_weight - desired_weight);
  NSInteger candidate_weight_delta_magnitude =
      abs(candidate_weight - desired_weight);

  // If both are the same distance from the desired weight, prefer the candidate
  // if it is further from medium.
  if (chosen_weight_delta_magnitude == candidate_weight_delta_magnitude)
    return abs(candidate_weight - 6) > abs(chosen_weight - 6);

  // Otherwise, prefer the one closer to the desired weight.
  return candidate_weight_delta_magnitude < chosen_weight_delta_magnitude;
}

CTFontSymbolicTraits ComputeDesiredTraits(FontSelectionValue desired_weight,
                                          FontSelectionValue desired_slant,
                                          FontSelectionValue desired_width) {
  CTFontSymbolicTraits traits = 0;
  if (desired_weight > kNormalWeightValue) {
    traits |= kCTFontTraitBold;
  }
  if (desired_slant != kNormalSlopeValue) {
    traits |= kCTFontTraitItalic;
  }
  if (desired_width > kNormalWidthValue) {
    traits |= kCTFontTraitExpanded;
  }
  if (desired_width < kNormalWidthValue) {
    traits |= kCTFontTraitCondensed;
  }
  return traits;
}

bool BetterChoiceCT(CTFontSymbolicTraits desired_traits,
                    int desired_weight,
                    CTFontSymbolicTraits chosen_traits,
                    int chosen_weight,
                    CTFontSymbolicTraits candidate_traits,
                    int candidate_weight) {
  // A list of the traits we care about.
  // The top item in the list is the worst trait to mismatch; if a font has this
  // and we didn't ask for it, we'd prefer any other font in the family.
  const CTFontSymbolicTraits kMasks[] = {
      kCTFontTraitCondensed, kCTFontTraitExpanded, kCTFontTraitItalic};

  for (CTFontSymbolicTraits mask : kMasks) {
    bool desired = (desired_traits & mask) != 0;
    bool chosen_has_unwanted_trait = desired != ((chosen_traits & mask) != 0);
    bool candidate_has_unwanted_trait =
        desired != ((candidate_traits & mask) != 0);
    if (!candidate_has_unwanted_trait && chosen_has_unwanted_trait) {
      return true;
    }
    if (!chosen_has_unwanted_trait && candidate_has_unwanted_trait) {
      return false;
    }
  }

  int chosen_weight_delta_magnitude = abs(chosen_weight - desired_weight);
  int candidate_weight_delta_magnitude = abs(candidate_weight - desired_weight);

  // If both are the same distance from the desired weight, prefer the candidate
  // if it is further from medium, i.e. 500.
  if (chosen_weight_delta_magnitude == candidate_weight_delta_magnitude) {
    return abs(candidate_weight - 500) > abs(chosen_weight - 500);
  }

  // Otherwise, prefer the one closer to the desired weight.
  return candidate_weight_delta_magnitude < chosen_weight_delta_magnitude;
}

ScopedCFTypeRef<CTFontRef> BestStyleMatchForFamily(
    ScopedCFTypeRef<CFStringRef> family_name,
    CTFontSymbolicTraits desired_traits,
    int desired_weight,
    float size) {
  ScopedCFTypeRef<CFMutableDictionaryRef> attributes(CFDictionaryCreateMutable(
      kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));
  CFDictionarySetValue(attributes.get(), kCTFontFamilyNameAttribute,
                       family_name.get());

  ScopedCFTypeRef<CTFontDescriptorRef> family_descriptor(
      CTFontDescriptorCreateWithAttributes(attributes.get()));
  ScopedCFTypeRef<CFMutableArrayRef> descriptors(
      CFArrayCreateMutable(kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks));
  CFArrayAppendValue(descriptors.get(), family_descriptor.get());

  ScopedCFTypeRef<CTFontCollectionRef> collection_from_family(
      CTFontCollectionCreateWithFontDescriptors(descriptors.get(), nullptr));

  ScopedCFTypeRef<CFArrayRef> fonts_in_family(
      CTFontCollectionCreateMatchingFontDescriptors(
          collection_from_family.get()));
  if (!fonts_in_family) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }

  ScopedCFTypeRef<CTFontRef> matched_font_in_family;
  CTFontSymbolicTraits chosen_traits;
  int chosen_weight;

  for (CFIndex i = 0; i < CFArrayGetCount(fonts_in_family.get()); ++i) {
    CTFontDescriptorRef descriptor = CFCast<CTFontDescriptorRef>(
        CFArrayGetValueAtIndex(fonts_in_family.get(), i));
    if (!descriptor) {
      continue;
    }

    int candidate_traits = kCTNormalTraitsValue;
    int candidate_weight = kNormalWeightValue;
    ScopedCFTypeRef<CFDictionaryRef> traits_dict(CFCast<CFDictionaryRef>(
        CTFontDescriptorCopyAttribute(descriptor, kCTFontTraitsAttribute)));
    if (traits_dict) {
      CFNumberRef candidate_symbolic_traits_num =
          GetValueFromDictionary<CFNumberRef>(traits_dict.get(),
                                              kCTFontSymbolicTrait);
      if (candidate_symbolic_traits_num) {
        CFNumberGetValue(candidate_symbolic_traits_num, kCFNumberIntType,
                         &candidate_traits);
      }

      CFNumberRef candidate_weight_num = GetValueFromDictionary<CFNumberRef>(
          traits_dict.get(), kCTFontWeightTrait);
      if (candidate_weight_num) {
        float candidate_ct_weight;
        CFNumberGetValue(candidate_weight_num, kCFNumberFloatType,
                         &candidate_ct_weight);
        candidate_weight = ToCSSFontWeight(candidate_ct_weight);
      }
    }

    if (!matched_font_in_family ||
        BetterChoiceCT(desired_traits, desired_weight, chosen_traits,
                       chosen_weight, candidate_traits, candidate_weight)) {
      matched_font_in_family.reset(
          CTFontCreateWithFontDescriptor(descriptor, size, nullptr));
      chosen_traits = candidate_traits;
      chosen_weight = candidate_weight;
    }
  }
  return matched_font_in_family;
}

}  // namespace

ScopedCFTypeRef<CTFontRef> MatchUniqueFont(const AtomicString& unique_font_name,
                                           float size) {
  // Note the header documentation: when matching, the system first searches for
  // fonts with its value as their PostScript name, then falls back to searching
  // for fonts with its value as their family name, and then falls back to
  // searching for fonts with its value as their display name.
  ScopedCFTypeRef<CFStringRef> desired_name(
      unique_font_name.Impl()->CreateCFString());
  ScopedCFTypeRef<CTFontRef> matched_font(
      CTFontCreateWithName(desired_name.get(), size, nullptr));
  DCHECK(matched_font);

  // CoreText will usually give us *something* but not always an exactly matched
  // font.
  ScopedCFTypeRef<CFStringRef> matched_postscript_name(
      CTFontCopyPostScriptName(matched_font.get()));
  ScopedCFTypeRef<CFStringRef> matched_full_font_name(
      CTFontCopyFullName(matched_font.get()));
  // If the found font does not match in PostScript name or full font name, it's
  // not the exact match that is required, so return nullptr.
  if (matched_postscript_name &&
      CFStringCompare(matched_postscript_name.get(), desired_name.get(),
                      kCFCompareCaseInsensitive) != kCFCompareEqualTo &&
      matched_full_font_name &&
      CFStringCompare(matched_full_font_name.get(), desired_name.get(),
                      kCFCompareCaseInsensitive) != kCFCompareEqualTo) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }

  return matched_font;
}

void ClampVariationValuesToFontAcceptableRange(
    ScopedCFTypeRef<CTFontRef> ct_font,
    FontSelectionValue& weight,
    FontSelectionValue& width) {
  ScopedCFTypeRef<CFArrayRef> all_axes(CTFontCopyVariationAxes(ct_font.get()));
  if (!all_axes) {
    return;
  }
  for (CFIndex i = 0; i < CFArrayGetCount(all_axes.get()); ++i) {
    CFDictionaryRef axis =
        CFCast<CFDictionaryRef>(CFArrayGetValueAtIndex(all_axes.get(), i));
    if (!axis) {
      continue;
    }

    CFNumberRef axis_id = GetValueFromDictionary<CFNumberRef>(
        axis, kCTFontVariationAxisIdentifierKey);
    if (!axis_id) {
      continue;
    }
    int axis_id_value;
    if (!CFNumberGetValue(axis_id, kCFNumberIntType, &axis_id_value)) {
      continue;
    }

    CFNumberRef axis_min_number = GetValueFromDictionary<CFNumberRef>(
        axis, kCTFontVariationAxisMinimumValueKey);
    double axis_min_value = 0.0;
    if (!axis_min_number ||
        !CFNumberGetValue(axis_min_number, kCFNumberDoubleType,
                          &axis_min_value)) {
      continue;
    }

    CFNumberRef axis_max_number = GetValueFromDictionary<CFNumberRef>(
        axis, kCTFontVariationAxisMaximumValueKey);
    double axis_max_value = 0.0;
    if (!axis_max_number ||
        !CFNumberGetValue(axis_max_number, kCFNumberDoubleType,
                          &axis_max_value)) {
      continue;
    }

    FontSelectionRange capabilities_range({FontSelectionValue(axis_min_value),
                                           FontSelectionValue(axis_max_value)});

    if (axis_id_value == kWeightTag && weight != kNormalWeightValue) {
      weight = capabilities_range.clampToRange(weight);
    }
    if (axis_id_value == kWidthTag && width != kNormalWidthValue) {
      width = capabilities_range.clampToRange(width);
    }
  }
}

ScopedCFTypeRef<CTFontRef> MatchSystemUIFont(FontSelectionValue desired_weight,
                                             FontSelectionValue desired_slant,
                                             FontSelectionValue desired_width,
                                             float size) {
  ScopedCFTypeRef<CTFontRef> ct_font(
      CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr));
  // CoreText should always return a system-ui font.
  DCHECK(ct_font);

  CTFontSymbolicTraits desired_traits = 0;

  if (desired_slant != kNormalSlopeValue) {
    desired_traits |= kCTFontItalicTrait;
  }

  if (desired_weight >= kBoldThreshold) {
    desired_traits |= kCTFontBoldTrait;
  }

  if (desired_traits) {
    ct_font.reset(CTFontCreateCopyWithSymbolicTraits(
        ct_font.get(), size, nullptr, desired_traits, desired_traits));
  }

  if (desired_weight == kNormalWeightValue &&
      desired_width == kNormalWidthValue) {
    return ct_font;
  }

  ClampVariationValuesToFontAcceptableRange(ct_font, desired_weight,
                                            desired_width);

  ScopedCFTypeRef<CFMutableDictionaryRef> variations(CFDictionaryCreateMutable(
      kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));

  auto add_axis_to_variations = [&variations](const FourCharCode tag,
                                              float desired_value,
                                              float normal_value) {
    if (desired_value != normal_value) {
      ScopedCFTypeRef<CFNumberRef> tag_number(
          CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &tag));
      ScopedCFTypeRef<CFNumberRef> value_number(CFNumberCreate(
          kCFAllocatorDefault, kCFNumberFloatType, &desired_value));
      CFDictionarySetValue(variations.get(), tag_number.get(),
                           value_number.get());
    }
  };
  add_axis_to_variations(kWeightTag, desired_weight, kNormalWeightValue);
  add_axis_to_variations(kWidthTag, desired_width, kNormalWidthValue);

  ScopedCFTypeRef<CFMutableDictionaryRef> attributes(CFDictionaryCreateMutable(
      kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));
  CFDictionarySetValue(attributes.get(), kCTFontVariationAttribute,
                       variations.get());

  ScopedCFTypeRef<CTFontDescriptorRef> var_font_desc(
      CTFontDescriptorCreateWithAttributes(attributes.get()));

  return ScopedCFTypeRef<CTFontRef>(CTFontCreateCopyWithAttributes(
      ct_font.get(), size, nullptr, var_font_desc.get()));
}

// For legacy reasons, we first attempt to find an
// exact match comparing the `desired_family_string` to the PostScript name of
// the installed fonts.  If that fails we then do a search based on the family
// names of the installed fonts.
ScopedCFTypeRef<CTFontRef> MatchFontFamily(
    const AtomicString& desired_family_string,
    FontSelectionValue desired_weight,
    FontSelectionValue desired_slant,
    FontSelectionValue desired_width,
    float size) {
  if (!desired_family_string) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }
  ScopedCFTypeRef<CFStringRef> desired_name(
      desired_family_string.Impl()->CreateCFString());

  // Due to the way we detect whether we can in-process load a font using
  // `CanLoadInProcess`, compare
  // third_party/blink/renderer/platform/fonts/mac/font_platform_data_mac.mm,
  // we cannot match the LastResort font on Mac.
  // TODO(crbug.com/1519877): We should allow matching LastResort font.
  if (CFStringCompare(desired_name.get(), CFSTR("LastResort"),
                      kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }

  ScopedCFTypeRef<CTFontRef> matched_font(
      CTFontCreateWithName(desired_name.get(), size, nullptr));
  // CoreText should give us *something* but not always an exactly matched font.
  DCHECK(matched_font);

  // We perform matching by PostScript name for legacy and compatibility reasons
  // (Safari also does it), although CSS specs do not require that, see
  // crbug.com/641861.
  ScopedCFTypeRef<CFStringRef> matched_postscript_name(
      CTFontCopyPostScriptName(matched_font.get()));
  ScopedCFTypeRef<CFStringRef> matched_family_name(
      CTFontCopyFamilyName(matched_font.get()));

  // If the found font does not match in PostScript name or font family name,
  // it's not the exact match that is required, so return nullptr.
  if (matched_postscript_name &&
      CFStringCompare(matched_postscript_name.get(), desired_name.get(),
                      kCFCompareCaseInsensitive) != kCFCompareEqualTo &&
      matched_family_name &&
      CFStringCompare(matched_family_name.get(), desired_name.get(),
                      kCFCompareCaseInsensitive) != kCFCompareEqualTo) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }

  CTFontSymbolicTraits desired_traits =
      ComputeDesiredTraits(desired_weight, desired_slant, desired_width);

  if (matched_postscript_name &&
      CFStringCompare(matched_postscript_name.get(), desired_name.get(),
                      kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(matched_font.get());
    // Matched a font by PostScript name that has desired traits, so we
    // can skip matching by family name.
    if ((desired_traits & traits) == desired_traits) {
      return matched_font;
    }
  }

  if (!matched_family_name) {
    return ScopedCFTypeRef<CTFontRef>(nullptr);
  }

  return BestStyleMatchForFamily(matched_family_name, desired_traits,
                                 desired_weight, size);
}

// Family name is somewhat of a misnomer here.  We first attempt to find an
// exact match comparing the desiredFamily to the PostScript name of the
// installed fonts.  If that fails we then do a search based on the family
// names of the installed fonts.
NSFont* MatchNSFontFamily(const AtomicString& desired_family_string,
                          NSFontTraitMask desired_traits,
                          FontSelectionValue desired_weight,
                          float size) {
  DCHECK_NE(desired_family_string, FontCache::LegacySystemFontFamily());

  NSString* desired_family = desired_family_string;
  NSFontManager* font_manager = NSFontManager.sharedFontManager;

  // From macOS 10.15 `NSFontManager.availableFonts` does not list certain
  // fonts that availableMembersOfFontFamily actually shows results for, for
  // example "Hiragino Kaku Gothic ProN" is not listed, only Hiragino Sans is
  // listed. We previously enumerated availableFontFamilies and looked for a
  // case-insensitive string match here, but instead, we can rely on
  // availableMembersOfFontFamily here to do a case-insensitive comparison, then
  // set available_family to desired_family if the result was not empty.
  // See https://crbug.com/1000542
  NSString* available_family = nil;
  NSArray* fonts_in_family =
      [font_manager availableMembersOfFontFamily:desired_family];
  if (fonts_in_family && fonts_in_family.count) {
    available_family = desired_family;
  }

  NSInteger app_kit_font_weight = ToAppKitFontWeight(desired_weight);
  if (!available_family) {
    // Match by PostScript name.
    NSEnumerator* available_fonts =
        font_manager.availableFonts.objectEnumerator;
    NSString* available_font;
    NSFont* name_matched_font = nil;
    NSFontTraitMask desired_traits_for_name_match =
        desired_traits | (app_kit_font_weight >= 7 ? NSBoldFontMask : 0);
    while ((available_font = [available_fonts nextObject])) {
      if ([desired_family caseInsensitiveCompare:available_font] ==
          NSOrderedSame) {
        name_matched_font = [NSFont fontWithName:available_font size:size];

        // Special case Osaka-Mono.  According to <rdar://problem/3999467>, we
        // need to treat Osaka-Mono as fixed pitch.
        if ([desired_family caseInsensitiveCompare:@"Osaka-Mono"] ==
                NSOrderedSame &&
            desired_traits_for_name_match == 0) {
          return name_matched_font;
        }

        NSFontTraitMask traits = [font_manager traitsOfFont:name_matched_font];
        if ((traits & desired_traits_for_name_match) ==
            desired_traits_for_name_match) {
          return [font_manager convertFont:name_matched_font
                               toHaveTrait:desired_traits_for_name_match];
        }

        available_family = name_matched_font.familyName;
        break;
      }
    }
  }

  // Found a family, now figure out what weight and traits to use.
  BOOL chose_font = false;
  NSInteger chosen_weight = 0;
  NSFontTraitMask chosen_traits = 0;
  NSString* chosen_full_name = nil;

  NSArray<NSArray*>* fonts =
      [font_manager availableMembersOfFontFamily:available_family];
  for (NSArray* font_info in fonts) {
    // Each font is represented by an array of four elements:

    // TODO(https://crbug.com/1442008): The docs say that font_info[0] is the
    // PostScript name of the font, but it's treated as the full name here.
    // Either the docs are wrong and we should note that here for future readers
    // of the code, or the docs are right and we should fix this.
    // https://developer.apple.com/documentation/appkit/nsfontmanager/1462316-availablemembersoffontfamily
    NSString* font_full_name = font_info[0];
    // font_info[1] is "the part of the font name used in the font panel that's
    // not the font name". This is not needed.
    NSInteger font_weight = [font_info[2] intValue];
    NSFontTraitMask font_traits = [font_info[3] unsignedIntValue];

    BOOL new_winner;
    if (!chose_font) {
      new_winner = AcceptableChoice(desired_traits, font_traits);
    } else {
      new_winner =
          BetterChoice(desired_traits, app_kit_font_weight, chosen_traits,
                       chosen_weight, font_traits, font_weight);
    }

    if (new_winner) {
      chose_font = YES;
      chosen_weight = font_weight;
      chosen_traits = font_traits;
      chosen_full_name = font_full_name;

      if (chosen_weight == app_kit_font_weight &&
          (chosen_traits & IMPORTANT_FONT_TRAITS) ==
              (desired_traits & IMPORTANT_FONT_TRAITS))
        break;
    }
  }

  if (!chose_font)
    return nil;

  NSFont* font = [NSFont fontWithName:chosen_full_name size:size];

  if (!font)
    return nil;

  if (RuntimeEnabledFeatures::MacFontsDeprecateFontTraitsWorkaroundEnabled()) {
    return font;
  }

  NSFontTraitMask actual_traits = 0;
  if (desired_traits & NSFontItalicTrait)
    actual_traits = [font_manager traitsOfFont:font];
  NSInteger actual_weight = [font_manager weightOfFont:font];

  bool synthetic_bold = app_kit_font_weight >= 7 && actual_weight < 7;
  bool synthetic_italic = (desired_traits & NSFontItalicTrait) &&
                          !(actual_traits & NSFontItalicTrait);

  // There are some malformed fonts that will be correctly returned by
  // -fontWithFamily:traits:weight:size: as a match for a particular trait,
  // though -[NSFontManager traitsOfFont:] incorrectly claims the font does not
  // have the specified trait. This could result in applying
  // synthetic bold on top of an already-bold font, as reported in
  // <http://bugs.webkit.org/show_bug.cgi?id=6146>. To work around this
  // problem, if we got an apparent exact match, but the requested traits
  // aren't present in the matched font, we'll try to get a font from the same
  // family without those traits (to apply the synthetic traits to later).
  NSFontTraitMask non_synthetic_traits = desired_traits;

  if (synthetic_bold)
    non_synthetic_traits &= ~NSBoldFontMask;

  if (synthetic_italic)
    non_synthetic_traits &= ~NSItalicFontMask;

  if (non_synthetic_traits != desired_traits) {
    NSFont* font_without_synthetic_traits =
        [font_manager fontWithFamily:available_family
                              traits:non_synthetic_traits
                              weight:chosen_weight
                                size:size];
    if (font_without_synthetic_traits)
      font = font_without_synthetic_traits;
  }

  return font;
}

int ToAppKitFontWeight(FontSelectionValue font_weight) {
  float weight = font_weight;
  if (weight <= 50 || weight >= 950)
    return 5;

  size_t select_weight = roundf(weight / 100) - 1;
  static int app_kit_font_weights[] = {
      2,   // FontWeight100
      3,   // FontWeight200
      4,   // FontWeight300
      5,   // FontWeight400
      6,   // FontWeight500
      8,   // FontWeight600
      9,   // FontWeight700
      10,  // FontWeight800
      12,  // FontWeight900
  };
  DCHECK_GE(select_weight, 0ul);
  DCHECK_LE(select_weight, std::size(app_kit_font_weights));
  return app_kit_font_weights[select_weight];
}

// CoreText font weight ranges are taken from `GetFontWeightFromCTFont` in
// `ui/gfx/platform_font_mac.mm`
int ToCSSFontWeight(float ct_font_weight) {
  constexpr struct {
    float weight_lower;
    float weight_upper;
    int css_weight;
  } weights[] = {
      {-1.0, -0.70, 100},   // Thin (Hairline)
      {-0.70, -0.45, 200},  // Extra Light (Ultra Light)
      {-0.45, -0.10, 300},  // Light
      {-0.10, 0.10, 400},   // Normal (Regular)
      {0.10, 0.27, 500},    // Medium
      {0.27, 0.35, 600},    // Semi Bold (Demi Bold)
      {0.35, 0.50, 700},    // Bold
      {0.50, 0.60, 800},    // Extra Bold (Ultra Bold)
      {0.60, 1.0, 900},     // Black (Heavy)
  };
  for (const auto& item : weights) {
    if (item.weight_lower <= ct_font_weight &&
        ct_font_weight <= item.weight_upper) {
      return item.css_weight;
    }
  }
  return kNormalWeightValue;
}

float ToCTFontWeight(int css_weight) {
  if (css_weight <= 50 || css_weight >= 950) {
    return 0.0;
  }
  const float weights[] = {
      -0.80,  // Thin (Hairline)
      -0.60,  // Extra Light (Ultra Light)
      -0.40,  // Light
      0.0,    // Normal (Regular)
      0.23,   // Medium
      0.30,   // Semi Bold (Demi Bold)
      0.40,   // Bold
      0.56,   // Extra Bold (Ultra Bold)
      0.62,   // Black (Heavy)
  };
  int index = (css_weight - 50) / 100;
  return weights[index];
}

}  // namespace blink
