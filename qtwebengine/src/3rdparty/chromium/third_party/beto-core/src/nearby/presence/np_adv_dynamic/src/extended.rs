// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use crypto_provider::CryptoProvider;
use np_adv::{extended::data_elements::*, extended::serialize::*, shared_data::*};
use sink::Sink;
use thiserror::Error;

/// An advertisement builder for V1 advertisements where the
/// presence/absence of salt is determined at run-time instead of compile-time.
pub struct BoxedAdvBuilder {
    adv_builder: AdvBuilder,
}

impl From<AdvBuilder> for BoxedAdvBuilder {
    fn from(adv_builder: AdvBuilder) -> Self {
        BoxedAdvBuilder { adv_builder }
    }
}

/// Error possibly generated when attempting to add a section to
/// a BoxedAdvBuilder.
#[derive(Debug, Error)]
pub enum BoxedAddSectionError {
    /// An error which was generated by the underlying AdvBuilder wrapped by the BoxedAdvBuilder
    #[error("{0}")]
    Underlying(AddSectionError),
    /// An error generated when the boxed advertisement builder is unsalted, but the section
    /// identity requires salt.
    #[error("Error generated when the BoxedAdvBuilder is unsalted, but the section identity requires salt.")]
    IdentityRequiresSaltError,
}

impl From<AddSectionError> for BoxedAddSectionError {
    fn from(wrapped: AddSectionError) -> Self {
        BoxedAddSectionError::Underlying(wrapped)
    }
}

fn wrap_section_builder<'a, C: CryptoProvider, S: Into<BoxedSectionBuilder<'a, C>>>(
    maybe_section_builder: Result<S, AddSectionError>,
) -> Result<BoxedSectionBuilder<'a, C>, BoxedAddSectionError> {
    let section_builder = maybe_section_builder?;
    Ok(section_builder.into())
}

impl BoxedAdvBuilder {
    /// Create a section builder using the given identity.
    ///
    /// Returns `Err` if the underlying advertisement builder
    /// yields an error when attempting to add a new section
    /// (typically because there's no more available adv space),
    /// or if the requested identity requires salt, and the
    /// advertisement builder is salt-less.
    pub fn section_builder<C: CryptoProvider>(
        &mut self,
        identity: BoxedIdentity<C>,
    ) -> Result<BoxedSectionBuilder<C>, BoxedAddSectionError> {
        match identity {
            BoxedIdentity::PublicIdentity => wrap_section_builder(
                self.adv_builder.section_builder(PublicSectionEncoder::default()),
            ),
            BoxedIdentity::MicEncrypted(ident) => {
                wrap_section_builder(self.adv_builder.section_builder(ident))
            }
            BoxedIdentity::SignedEncrypted(ident) => {
                wrap_section_builder(self.adv_builder.section_builder(ident))
            }
        }
    }

    /// Convert the builder into an encoded advertisement.
    pub fn into_advertisement(self) -> EncodedAdvertisement {
        self.adv_builder.into_advertisement()
    }
}

/// A wrapped v1 identity whose type is given at run-time.
pub enum BoxedIdentity<C: CryptoProvider> {
    /// Public identity.
    PublicIdentity,
    /// An encrypted identity leveraging MIC for verification.
    MicEncrypted(MicEncryptedSectionEncoder<C>),
    /// An encrypted identity leveraging signatures for verification.
    SignedEncrypted(SignedEncryptedSectionEncoder<C>),
}

/// A `SectionBuilder` whose corresponding Identity
/// and salted-ness is given at run-time instead of
/// at compile-time.
pub enum BoxedSectionBuilder<'a, C: CryptoProvider> {
    /// A builder for a public section.
    Public(SectionBuilder<'a, PublicSectionEncoder>),
    /// A builder for a MIC-verified section.
    MicEncrypted(SectionBuilder<'a, MicEncryptedSectionEncoder<C>>),
    /// A builder for a signature-verified section.
    SignedEncrypted(SectionBuilder<'a, SignedEncryptedSectionEncoder<C>>),
}

impl<'a, C: CryptoProvider> BoxedSectionBuilder<'a, C> {
    /// Returns true if this wraps a section builder which
    /// leverages some encrypted identity.
    pub fn is_encrypted(&self) -> bool {
        match self {
            BoxedSectionBuilder::Public(_) => false,
            BoxedSectionBuilder::MicEncrypted(_) => true,
            BoxedSectionBuilder::SignedEncrypted(_) => true,
        }
    }
    /// Add this builder to the advertisement that created it.
    pub fn add_to_advertisement(self) {
        match self {
            BoxedSectionBuilder::Public(x) => x.add_to_advertisement(),
            BoxedSectionBuilder::MicEncrypted(x) => x.add_to_advertisement(),
            BoxedSectionBuilder::SignedEncrypted(x) => x.add_to_advertisement(),
        }
    }
    /// Add a data element to the section with a closure that returns a `Result`.
    ///
    /// The provided `build_de` closure will be invoked with the derived salt for this DE,
    /// if any salt has been specified for the surrounding advertisement.
    pub fn add_de_res<E>(
        &mut self,
        build_de: impl FnOnce(Option<DeSalt<C>>) -> Result<BoxedWriteDataElement, E>,
    ) -> Result<(), AddDataElementError<E>> {
        match self {
            BoxedSectionBuilder::Public(x) => {
                let build_de_modified = |()| build_de(None);
                x.add_de_res(build_de_modified)
            }
            BoxedSectionBuilder::MicEncrypted(x) => {
                let build_de_modified = |de_salt: DeSalt<C>| build_de(Some(de_salt));
                x.add_de_res(build_de_modified)
            }
            BoxedSectionBuilder::SignedEncrypted(x) => {
                let build_de_modified = |de_salt: DeSalt<C>| build_de(Some(de_salt));
                x.add_de_res(build_de_modified)
            }
        }
    }
    /// Like add_de_res, but for infalliable closures
    pub fn add_de(
        &mut self,
        build_de: impl FnOnce(Option<DeSalt<C>>) -> BoxedWriteDataElement,
    ) -> Result<(), AddDataElementError<()>> {
        self.add_de_res(|derived_salt| Ok::<_, ()>(build_de(derived_salt)))
    }
}

impl<'a, C: CryptoProvider> From<SectionBuilder<'a, PublicSectionEncoder>>
    for BoxedSectionBuilder<'a, C>
{
    fn from(section_builder: SectionBuilder<'a, PublicSectionEncoder>) -> Self {
        BoxedSectionBuilder::Public(section_builder)
    }
}

impl<'a, C: CryptoProvider> From<SectionBuilder<'a, MicEncryptedSectionEncoder<C>>>
    for BoxedSectionBuilder<'a, C>
{
    fn from(section_builder: SectionBuilder<'a, MicEncryptedSectionEncoder<C>>) -> Self {
        BoxedSectionBuilder::MicEncrypted(section_builder)
    }
}

impl<'a, C: CryptoProvider> From<SectionBuilder<'a, SignedEncryptedSectionEncoder<C>>>
    for BoxedSectionBuilder<'a, C>
{
    fn from(section_builder: SectionBuilder<'a, SignedEncryptedSectionEncoder<C>>) -> Self {
        BoxedSectionBuilder::SignedEncrypted(section_builder)
    }
}

/// Mutable trait object reference to a `Sink<u8>`
pub struct DynSink<'a> {
    wrapped: &'a mut dyn Sink<u8>,
}

impl<'a> Sink<u8> for DynSink<'a> {
    fn try_extend_from_slice(&mut self, items: &[u8]) -> Option<()> {
        self.wrapped.try_extend_from_slice(items)
    }
    fn try_push(&mut self, item: u8) -> Option<()> {
        self.wrapped.try_push(item)
    }
}

impl<'a> From<&'a mut dyn Sink<u8>> for DynSink<'a> {
    fn from(wrapped: &'a mut dyn Sink<u8>) -> Self {
        DynSink { wrapped }
    }
}

/// A version of the WriteDataElement trait which is object-safe
pub trait DynWriteDataElement {
    /// Gets the data-element header for the data element
    fn de_header(&self) -> DeHeader;
    /// Writes the contents of the DE payload to the given DynSink.
    /// Returns Some(()) if the write operation was successful,
    /// and None if it was unsuccessful
    fn write_de_contents(&self, sink: DynSink) -> Option<()>;
}

impl<T: WriteDataElement> DynWriteDataElement for T {
    fn de_header(&self) -> DeHeader {
        WriteDataElement::de_header(self)
    }
    fn write_de_contents(&self, mut sink: DynSink) -> Option<()> {
        WriteDataElement::write_de_contents(self, &mut sink)
    }
}

/// Trait object wrapper for DynWriteDataElement instances
pub struct BoxedWriteDataElement {
    wrapped: Box<dyn DynWriteDataElement>,
}

impl BoxedWriteDataElement {
    /// Constructs a new `BoxedWriteDataElement` from a `WriteDataElement`
    /// whose trait impl is valid for a `'static` lifetime.
    pub fn new<D: WriteDataElement + 'static>(wrapped: D) -> Self {
        let wrapped = Box::new(wrapped);
        Self { wrapped }
    }
}

impl WriteDataElement for BoxedWriteDataElement {
    fn de_header(&self) -> DeHeader {
        self.wrapped.de_header()
    }
    fn write_de_contents<S: Sink<u8>>(&self, sink: &mut S) -> Option<()> {
        let sink: &mut dyn Sink<u8> = sink;
        let dyn_sink = DynSink::from(sink);
        self.wrapped.write_de_contents(dyn_sink)
    }
}

impl From<TxPower> for BoxedWriteDataElement {
    fn from(tx_power: TxPower) -> Self {
        BoxedWriteDataElement::new::<TxPowerDataElement>(tx_power.into())
    }
}

impl From<ContextSyncSeqNum> for BoxedWriteDataElement {
    fn from(context_sync_sequence_num: ContextSyncSeqNum) -> Self {
        BoxedWriteDataElement::new::<ContextSyncSeqNumDataElement>(context_sync_sequence_num.into())
    }
}

impl From<ActionsDataElement> for BoxedWriteDataElement {
    fn from(data: ActionsDataElement) -> Self {
        BoxedWriteDataElement::new(data)
    }
}
