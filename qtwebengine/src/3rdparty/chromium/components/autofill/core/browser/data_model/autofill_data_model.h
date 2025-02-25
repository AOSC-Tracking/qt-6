// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_DATA_MODEL_AUTOFILL_DATA_MODEL_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_DATA_MODEL_AUTOFILL_DATA_MODEL_H_

#include <stddef.h>

#include "base/time/time.h"
#include "components/autofill/core/browser/data_model/form_group.h"

namespace autofill {

struct AutofillMetadata;

// This class is an interface for the primary data models that back Autofill.
// The information in objects of this class is managed by the
// PersonalDataManager.
class AutofillDataModel : public FormGroup {
 public:
  // TODO(crbug.com/1174203): Remove.
  enum class ValidityState {
    // The field has not been validated.
    kUnvalidated = 0,
    // The field is empty.
    kEmpty = 1,
    // The field is valid.
    kValid = 2,
    // The field is invalid.
    kInvalid = 3,
    // The validation for the field is unsupported.
    kUnsupported = 4,
  };

  AutofillDataModel();
  ~AutofillDataModel() override;

  // Calculates the number of days since the model was last used by subtracting
  // the model's last recent |use_date_| from the |current_time|.
  int GetDaysSinceLastUse(base::Time current_time) const;

  size_t use_count() const { return use_count_; }
  void set_use_count(size_t count) { use_count_ = count; }

  // Writing in and reading from database converts dates between time_t and
  // Time, therefore the microseconds get lost. Therefore, we need to round the
  // dates to seconds for both |use_date_| and |modification_date_|.
  const base::Time& use_date() const { return use_date_; }
  void set_use_date(const base::Time& time) { use_date_ = time; }

  bool UseDateEqualsInSeconds(const AutofillDataModel* other) const;

  const base::Time& modification_date() const { return modification_date_; }
  void set_modification_date(const base::Time& time) {
    modification_date_ = time;
  }

  // Compares two data models according to their ranking score. The score uses
  // a combination of use count and days since last use to determine the
  // relevance of the profile. A greater ranking score corresponds to a higher
  // ranking on the suggestion list.
  // The function defines a strict weak ordering that can be used for sorting.
  // Since data models can have the same score, it doesn't define a total order.
  // `comparison_time_` allows consistent sorting throughout the comparisons.
  bool HasGreaterRankingThan(const AutofillDataModel* other,
                             base::Time comparison_time) const;

  // Gets the metadata associated with this autofill data model.
  virtual AutofillMetadata GetMetadata() const;

  // Sets the |use_count_| and |use_date_| of this autofill data model. Returns
  // whether the metadata was set.
  virtual bool SetMetadata(const AutofillMetadata& metadata);

  // Returns whether the data model is deletable: if it has not been used for
  // longer than |kDisusedCreditCardDeletionTimeDelta|.
  virtual bool IsDeletable() const;

 protected:
  // Calculate the ranking score of a card or profile depending on their use
  // count and most recent use date.
  virtual double GetRankingScore(base::Time current_time) const;

 private:
  // The number of times this model has been used.
  size_t use_count_;

  // The last time the model was used, rounded in seconds. Any change should
  // use set_previous_use_date()
  base::Time use_date_;

  // The last time data in the model was modified, rounded in seconds. Any
  // change should use set_previous_modification_date()
  base::Time modification_date_;
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_DATA_MODEL_AUTOFILL_DATA_MODEL_H_
