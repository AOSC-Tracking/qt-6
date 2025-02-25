// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CONTENT_RENDERER_AUTOFILL_RENDERER_TEST_H_
#define COMPONENTS_AUTOFILL_CONTENT_RENDERER_AUTOFILL_RENDERER_TEST_H_

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "components/autofill/content/common/mojom/autofill_driver.mojom.h"
#include "components/autofill/content/renderer/autofill_agent.h"
#include "content/public/test/render_view_test.h"
#include "mojo/public/cpp/bindings/associated_receiver_set.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace autofill::test {

class MockAutofillDriver : public mojom::AutofillDriver {
 public:
  MockAutofillDriver();
  ~MockAutofillDriver() override;

  void BindPendingReceiver(mojo::ScopedInterfaceEndpointHandle handle) {
    receivers_.Add(this, mojo::PendingAssociatedReceiver<mojom::AutofillDriver>(
                             std::move(handle)));
  }

  MOCK_METHOD(void,
              SetFormToBeProbablySubmitted,
              (const std::optional<FormData>& form),
              (override));
  MOCK_METHOD(void,
              FormsSeen,
              (const std::vector<FormData>& updated_forms,
               const std::vector<FormRendererId>& removed_forms),
              (override));
  MOCK_METHOD(void,
              FormSubmitted,
              (const FormData& form,
               bool known_success,
               mojom::SubmissionSource source),
              (override));
  MOCK_METHOD(void,
              TextFieldDidChange,
              (const FormData& form,
               const FormFieldData& field,
               const gfx::RectF& bounding_box,
               base::TimeTicks timestamp),
              (override));
  MOCK_METHOD(void,
              TextFieldDidScroll,
              (const FormData& form,
               const FormFieldData& field,
               const gfx::RectF& bounding_box),
              (override));
  MOCK_METHOD(void,
              SelectControlDidChange,
              (const FormData& form,
               const FormFieldData& field,
               const gfx::RectF& bounding_box),
              (override));
  MOCK_METHOD(void,
              SelectOrSelectListFieldOptionsDidChange,
              (const FormData& form),
              (override));
  MOCK_METHOD(void,
              JavaScriptChangedAutofilledValue,
              (const FormData& form,
               const FormFieldData& field,
               const std::u16string& old_value),
              (override));
  MOCK_METHOD(void,
              AskForValuesToFill,
              (const FormData& form,
               const FormFieldData& field,
               const gfx::RectF& bounding_box,
               AutofillSuggestionTriggerSource trigger_source),
              (override));
  MOCK_METHOD(void, HidePopup, (), (override));
  MOCK_METHOD(void,
              FocusNoLongerOnForm,
              (bool had_interacted_form),
              (override));
  MOCK_METHOD(void,
              FocusOnFormField,
              (const FormData& form,
               const FormFieldData& field,
               const gfx::RectF& bounding_box),
              (override));
  MOCK_METHOD(void,
              DidFillAutofillFormData,
              (const FormData& form, base::TimeTicks timestamp),
              (override));
  MOCK_METHOD(void, DidEndTextFieldEditing, (), (override));

 private:
  mojo::AssociatedReceiverSet<mojom::AutofillDriver> receivers_;
};

class AutofillRendererTest : public content::RenderViewTest {
 public:
  AutofillRendererTest();
  ~AutofillRendererTest() override;

  // content::RenderViewTest:
  void SetUp() override;
  void TearDown() override;

  // Simulates a click on the element with id `element_id` and, if, successful,
  // runs until the task environment is idle. Waits until the `TaskEnvironment`
  // is idle to ensure that the `AutofillDriver` is notified via mojo.
  bool SimulateElementClickAndWait(const std::string& element_id);

  // Simulate focusing an element without clicking it. Waits until the
  // `TaskEnvironment` is idle to ensure that the `AutofillDriver` is notified
  // via mojo.
  void SimulateElementFocusAndWait(std::string_view element_id);

  // AutofillDriver::FormsSeen() is throttled indirectly because some callsites
  // of AutofillAgent::ProcessForms() are throttled. This function blocks until
  // FormsSeen() has happened.
  void WaitForFormsSeen() {
    task_environment_.FastForwardBy(AutofillAgent::kFormsSeenThrottle * 3 / 2);
  }

 protected:
  AutofillAgent& autofill_agent() { return *autofill_agent_; }
  MockAutofillDriver& autofill_driver() { return autofill_driver_; }

 private:
  ::testing::NiceMock<MockAutofillDriver> autofill_driver_;
  blink::AssociatedInterfaceRegistry associated_interfaces_;
  std::unique_ptr<AutofillAgent> autofill_agent_;
};

}  // namespace autofill::test

#endif  // COMPONENTS_AUTOFILL_CONTENT_RENDERER_AUTOFILL_RENDERER_TEST_H_
