// Copyright 2022 Google LLC
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

#![allow(clippy::unwrap_used)]

extern crate std;
use super::*;
use crate::extended::deserialize::encrypted_section::{
    MicEncryptedSection, SignatureEncryptedSection,
};
use crate::extended::deserialize::test_stubs::IntermediateSectionExt;
use rand::rngs::StdRng;
use rand::seq::SliceRandom;
use rand::{Rng, SeedableRng};
use std::vec;

#[test]
fn parse_adv_ext_public_identity() {
    // 2 sections, 3 DEs each
    let mut adv_body = vec![];
    // section
    adv_body.push(10);
    // public identity
    adv_body.push(0x03);
    // de 1 byte header, type 5, len 5
    adv_body.extend_from_slice(&[0x55, 0x01, 0x02, 0x03, 0x04, 0x05]);
    // de 2 byte header, type 6, len 1
    adv_body.extend_from_slice(&[0x81, 0x06, 0x01]);

    let parsed_sections =
        parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap().into_vec();
    assert_eq!(
        vec![IntermediateSection::from(PlaintextSection::new(
            PlaintextIdentityMode::Public,
            SectionContents::new(10, &adv_body[2..], 1)
        ))],
        parsed_sections,
    );
    let expected_des = [
        // 1 byte header, len 5
        DataElement {
            offset: 1_u8.into(),
            de_type: 5_u8.into(),
            contents: &[0x01, 0x02, 0x03, 0x04, 0x05],
        },
        // 2 byte header, len 1
        DataElement { offset: 2_u8.into(), de_type: 6_u8.into(), contents: &[0x01] },
    ];

    assert_eq!(
        &expected_des[..],
        &parsed_sections[0].as_plaintext().unwrap().collect_data_elements().unwrap()
    );
}

#[test]
fn parse_adv_ext_identity() {
    // 3 sections
    let mut adv_body = vec![];
    // section - 48 bytes total
    adv_body.push(19 + 18 + 10);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x08, // scheme (signature)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // private identity -- 2 + 16x 0x33
    adv_body.extend_from_slice(&[
        0x90, 0x01, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x33, 0x33,
    ]);
    // 10 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 10]);

    // section - 49 bytes total
    adv_body.push(19 + 18 + 11);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x08, // scheme (signature)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // trusted identity -- 2 + 16x 0x55
    adv_body.extend_from_slice(&[
        0x90, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55,
    ]);
    // 11 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 11]);

    // section - 50 bytes total
    adv_body.push(19 + 18 + 12);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x08, // scheme (signature)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // provisioned identity -- 2 + 16x 0x77
    adv_body.extend_from_slice(&[
        0x90, 0x04, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
        0x77, 0x77, 0x77,
    ]);
    // 12 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 12]);

    let adv_header = V1Header { header_byte: 0x20 };
    let encryption_info = EncryptionInfo {
        bytes: [
            0x91, 0x10, // header bytes
            0x08, // scheme (signature)
            0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
            0x11, 0x11,
        ],
    };
    let expected_sections = [
        SignatureEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 47,
                adv_header,
                encryption_info: encryption_info.clone(),
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x01],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Private,
                },
                // skip section header + encryption info + identity header -> end of section
                all_ciphertext: &adv_body[1 + 19 + 2..48],
            },
        },
        SignatureEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 48,
                adv_header,
                encryption_info: encryption_info.clone(),
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x02],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Trusted,
                },
                all_ciphertext: &adv_body[48 + 1 + 19 + 2..97],
            },
        },
        SignatureEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 49,
                adv_header,
                encryption_info,
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x04],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Provisioned,
                },
                all_ciphertext: &adv_body[97 + 1 + 19 + 2..],
            },
        },
    ];
    let parsed_sections = parse_sections(adv_header, &adv_body).unwrap();
    assert_eq!(parsed_sections.into_vec(), expected_sections.map(IntermediateSection::from));
}

#[test]
fn parse_adv_ext_mic_identity() {
    // 3 sections
    let mut adv_body = vec![];
    // section - 64 bytes total
    adv_body.push(19 + 18 + 10 + 16);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x00, // scheme (mic)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // private identity -- 2 + 16x 0x55
    adv_body.extend_from_slice(&[
        0x90, 0x01, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55,
    ]);
    // 10 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 10]);
    // mic - 16x 0x33
    adv_body.extend_from_slice(&[
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33,
    ]);

    // section - 65 bytes total
    adv_body.push(19 + 18 + 11 + 16);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x00, // scheme (mic)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // trusted identity -- 2 + 16x 0x77
    adv_body.extend_from_slice(&[
        0x90, 0x02, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
        0x77, 0x77, 0x77,
    ]);
    // 11 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 11]);
    // mic - 16x 0x66
    adv_body.extend_from_slice(&[
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
        0x66,
    ]);

    // section - 66 bytes total
    adv_body.push(19 + 18 + 12 + 16);
    // encryption info -- 2 + 1 + 16x 0x11
    adv_body.extend_from_slice(&[
        0x91, 0x10, // header
        0x00, // scheme (mic)
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, // salt
    ]);
    // provisioned identity -- 2 + 16x 0xAA
    adv_body.extend_from_slice(&[
        0x90, 0x04, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA,
    ]);
    // 12 bytes of 0xFF ciphertext
    adv_body.extend_from_slice(&[0xFF; 12]);
    // mic - 16x 0x99
    adv_body.extend_from_slice(&[
        0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
        0x99,
    ]);

    let adv_header = V1Header { header_byte: 0x20 };
    let encryption_info = EncryptionInfo {
        bytes: [
            0x91, 0x10, // header bytes
            0x00, // scheme (mic)
            0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
            0x11, 0x11,
        ],
    };
    let expected_sections = [
        MicEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 63,
                adv_header,
                encryption_info: encryption_info.clone(),
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x01],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Private,
                },
                // skip section header +  encryption info + identity header -> end of ciphertext
                all_ciphertext: &adv_body[1 + 19 + 2..64 - 16],
            },
            mic: SectionMic::from([0x33; 16]),
        },
        MicEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 64,
                adv_header,
                encryption_info: encryption_info.clone(),
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x02],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Trusted,
                },
                all_ciphertext: &adv_body[64 + 1 + 19 + 2..129 - 16],
            },
            mic: SectionMic::from([0x66; 16]),
        },
        MicEncryptedSection {
            contents: EncryptedSectionContents {
                section_header: 65,
                adv_header,
                encryption_info,
                identity: EncryptedIdentityMetadata {
                    header_bytes: [0x90, 0x04],
                    offset: 1_u8.into(),
                    identity_type: EncryptedIdentityDataElementType::Provisioned,
                },
                all_ciphertext: &adv_body[129 + 1 + 19 + 2..195 - 16],
            },
            mic: SectionMic::from([0x99; 16]),
        },
    ];
    let parsed_sections = parse_sections(adv_header, &adv_body).unwrap();
    assert_eq!(parsed_sections.into_vec(), &expected_sections.map(IntermediateSection::from));
}

#[test]
fn parse_de_with_1_byte_header() {
    let data = [0x51, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[6..],
            ProtoDataElement {
                header: DeHeader {
                    de_type: 1_u8.into(),
                    header_bytes: ArrayView::try_from_slice(&[0x51]).unwrap(),
                    contents_len: 5_u8.try_into().unwrap()
                },
                contents: &data[1..6]
            }
        )),
        ProtoDataElement::parse(&data)
    );
}

#[test]
fn parse_de_with_2_byte_header() {
    let data = [0x85, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[7..],
            ProtoDataElement {
                header: DeHeader {
                    de_type: 1_u8.into(),
                    header_bytes: ArrayView::try_from_slice(&[0x85, 0x01]).unwrap(),
                    contents_len: 5_u8.try_into().unwrap()
                },
                contents: &data[2..7]
            }
        )),
        ProtoDataElement::parse(&data)
    );
}

#[test]
fn parse_de_with_3_byte_header() {
    let data = [0x85, 0xC1, 0x41, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[8..],
            ProtoDataElement {
                header: DeHeader {
                    header_bytes: ArrayView::try_from_slice(&[0x85, 0xC1, 0x41]).unwrap(),
                    contents_len: 5_u8.try_into().unwrap(),
                    de_type: 0b0000_0000_0000_0000_0010_0000_1100_0001_u32.into(),
                },
                contents: &data[3..8]
            }
        )),
        ProtoDataElement::parse(&data)
    );
}

#[test]
fn parse_de_header_1_byte() {
    let data = [0x51, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[1..],
            DeHeader {
                de_type: 1_u8.into(),
                contents_len: 5_u8.try_into().unwrap(),
                header_bytes: ArrayView::try_from_slice(&[0x51]).unwrap(),
            }
        )),
        DeHeader::parse(&data)
    );
}

#[test]
fn parse_de_header_2_bytes() {
    let data = [0x83, 0x01, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[2..],
            DeHeader {
                de_type: 1_u8.into(),
                contents_len: 3_u8.try_into().unwrap(),
                header_bytes: ArrayView::try_from_slice(&[0x83, 0x01]).unwrap(),
            }
        )),
        DeHeader::parse(&data)
    );
}

#[test]
fn parse_de_header_3_bytes() {
    let data = [0x83, 0xC1, 0x41, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[3..],
            DeHeader {
                de_type: 0b0000_0000_0000_0000_0010_0000_1100_0001_u32.into(),
                contents_len: 3_u8.try_into().unwrap(),
                header_bytes: ArrayView::try_from_slice(&[0x83, 0xC1, 0x41]).unwrap(),
            }
        )),
        DeHeader::parse(&data)
    );
}

#[test]
fn parse_de_header_4_bytes() {
    let data = [0x83, 0xC1, 0xC1, 0x41, 0xFF, 0xFF];
    assert_eq!(
        Ok((
            &data[4..],
            DeHeader {
                de_type: 0b0000_0000_0001_0000_0110_0000_1100_0001_u32.into(),
                contents_len: 3_u8.try_into().unwrap(),
                header_bytes: ArrayView::try_from_slice(&[0x83, 0xC1, 0xC1, 0x41]).unwrap(),
            }
        )),
        DeHeader::parse(&data)
    );
}

#[test]
fn public_identity_not_first_de_error() {
    let mut adv_body = vec![];
    // section
    adv_body.push(3 + 1);
    // misc other DE
    adv_body.extend_from_slice(&[0x81, 0x70, 0xFF]);
    // public identity after another DE
    adv_body.push(0x03);

    assert_eq!(
        nom::Err::Error(error::Error { input: &adv_body[1..], code: error::ErrorKind::Verify }),
        parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap_err()
    );
}

#[test]
fn invalid_public_section() {
    let mut rng = StdRng::from_entropy();
    for _ in 0..100 {
        let mut adv_body = vec![];
        // Add section length
        let remove_section_len = rng.gen_bool(0.5);
        // Add public identity
        let add_public_identity = rng.gen_bool(0.5);
        // Add DEs
        let add_des = rng.gen_bool(0.5);
        // Shuffle adv
        let shuffle = rng.gen_bool(0.5);

        adv_body.push(0);
        if add_public_identity {
            adv_body[0] += 1;
            adv_body.push(0x03);
        }
        if add_des {
            adv_body[0] += 1;
            adv_body.extend_from_slice(&[0x81]);
        }
        if remove_section_len {
            let _ = adv_body.remove(0);
        }

        let ordered_adv = adv_body.clone();

        if shuffle {
            adv_body.shuffle(&mut rng);
        }
        // A V1 public section is invalid if
        // * section length is missing
        // * the section is empty
        // * the section identity is missing
        // * the identity does not follow the section length
        // * the section length is 0
        if remove_section_len || !add_public_identity || (ordered_adv != adv_body) {
            let _ = parse_sections(V1Header { header_byte: 0x20 }, &adv_body)
                .expect_err("Expected to fail because of missing section length or identity");
        }
    }
}

// There can only be one identity DE
#[test]
fn public_identity_after_public_identity_error() {
    let mut adv_body = vec![];
    // section
    adv_body.push(1 + 3 + 1);
    // public identity after another DE
    adv_body.push(0x03);
    // misc other DE
    adv_body.extend_from_slice(&[0x81, 0x70, 0xFF]);
    // public identity after another DE
    adv_body.push(0x03);

    let sections = parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap();
    assert_eq!(sections.len(), 1);
    assert_eq!(
        DataElementParseError::DuplicateIdentityDataElement,
        sections[0].as_plaintext().unwrap().collect_data_elements().unwrap_err()
    );
}

#[test]
fn salt_public_identity_error() {
    let mut adv_body = vec![];
    // section
    adv_body.push(3 + 1 + 3);
    // salt - 1 + 2x 0x22 (invalid: must be first DE)
    adv_body.extend_from_slice(&[0x20, 0x22, 0x22]);
    // public identity
    adv_body.push(0x03);
    // misc other DE
    adv_body.extend_from_slice(&[0x81, 0x70, 0xFF]);

    assert_eq!(
        nom::Err::Error(error::Error {
            input: &adv_body[1..],
            // Eof because all_consuming is used to ensure complete section is parsed
            code: error::ErrorKind::Verify
        }),
        parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap_err()
    );
}

#[test]
fn salt_mic_public_identity_error() {
    let mut adv_body = vec![];
    // section
    adv_body.push(3 + 18 + 1 + 3);
    // salt - 1 + 2x 0x22 (invalid: must be first DE)
    adv_body.extend_from_slice(&[0x20, 0x22, 0x22]);
    // mic - 2 + 16x 0x33
    adv_body.extend_from_slice(&[
        0x90, 0x13, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x33, 0x33,
    ]);
    // public identity
    adv_body.push(0x03);
    // misc other DE
    adv_body.extend_from_slice(&[0x81, 0x70, 0xFF]);

    assert_eq!(
        nom::Err::Error(error::Error {
            input: &adv_body[1..],
            // Eof because all_consuming is used to ensure complete section is parsed
            code: error::ErrorKind::Verify
        }),
        parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap_err()
    );
}

#[test]
fn parse_adv_no_identity() {
    let adv_body = vec![0x55, 0x01, 0x02, 0x03, 0x04, 0x05];
    assert_eq!(
        nom::Err::Error(error::Error { input: &adv_body[1..], code: error::ErrorKind::Eof }),
        parse_sections(V1Header { header_byte: 0x20 }, &adv_body).unwrap_err()
    );
}

#[test]
fn parse_empty_section() {
    // empty section - should return an EOF error
    let input = [];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read section contents
            input: input.as_slice(),
            code: error::ErrorKind::Eof
        }),
        IntermediateSection::parser_encrypted_with_header(V1Header { header_byte: 0x20 })(&input)
            .unwrap_err()
    );
}

#[test]
fn parse_de_header_non_canonical_multi_byte() {
    // length 1, type 1
    // first byte of type doesn't have any bits in it so it contributes nothing
    let input = [0b1000_0001, 0b1000_0000, 0b0000_0001];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read first type byte
            input: &input.as_slice()[1..],
            code: error::ErrorKind::Verify
        }),
        DeHeader::parse(&input).unwrap_err()
    );
}

#[test]
fn parse_section_length_zero() {
    // Section length of 0 - should return a verification error
    let input = [0x00];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read section contents
            input: input.as_slice(),
            code: error::ErrorKind::Verify
        }),
        IntermediateSection::parser_encrypted_with_header(V1Header { header_byte: 0x20 })(&input)
            .unwrap_err()
    );
}

#[test]
fn parse_section_length_overrun() {
    // section of length 0xF0 - legal but way longer than 3
    let input = [0xF0, 0x01, 0x02, 0x03];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read section contents
            input: &input.as_slice()[1..],
            code: error::ErrorKind::Eof
        }),
        IntermediateSection::parser_encrypted_with_header(V1Header { header_byte: 0x20 })(&input)
            .unwrap_err()
    );
}

#[test]
fn parse_de_single_byte_header_length_overrun() {
    // length 7, type 0x03
    let input = [0b0111_0011, 0x01, 0x02];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read DE contents
            input: &input.as_slice()[1..],
            code: error::ErrorKind::Eof
        }),
        ProtoDataElement::parse(&input).unwrap_err()
    );
}

#[test]
fn parse_de_multi_byte_header_length_overrun() {
    // length 7, type 0x0F
    let input = [0b1000_0111, 0x0F, 0x01, 0x02];
    assert_eq!(
        nom::Err::Error(error::Error {
            // attempted to read DE contents
            input: &input.as_slice()[2..],
            code: error::ErrorKind::Eof
        }),
        ProtoDataElement::parse(&input).unwrap_err()
    );
}

#[test]
fn parse_adv_signature_encrypted_plaintext_mix() {
    // 2 sections
    let mut adv_body = vec![];

    // section 1 - plaintext - 10 bytes
    adv_body.push(10);
    // public identity
    adv_body.push(0x03);
    // de 1 byte header, type 5, len 5
    adv_body.extend_from_slice(&[0x55, 0x01, 0x02, 0x03, 0x04, 0x05]);
    // de 2 byte header, type 6, len 1
    adv_body.extend_from_slice(&[0x81, 0x06, 0x01]);

    // section 2 - plaintext - 10 bytes
    adv_body.push(10);
    // public identity
    adv_body.push(0x03);
    // de 1 byte header, type 5, len 5
    adv_body.extend_from_slice(&[0x55, 0x01, 0x02, 0x03, 0x04, 0x05]);
    // de 2 byte header, type 6, len 1
    adv_body.extend_from_slice(&[0x81, 0x06, 0x01]);

    let adv_header = V1Header { header_byte: 0x20 };

    assert_eq!(
        nom::Err::Error(error::Error { input: &adv_body[11..], code: error::ErrorKind::Eof }),
        parse_sections(adv_header, &adv_body).unwrap_err()
    );
}

// for convenient .into() in expected test data

impl<'a> From<SignatureEncryptedSection<'a>> for IntermediateSection<'a> {
    fn from(s: SignatureEncryptedSection<'a>) -> Self {
        IntermediateSection::Ciphertext(CiphertextSection::SignatureEncryptedIdentity(s))
    }
}

impl<'a> From<MicEncryptedSection<'a>> for IntermediateSection<'a> {
    fn from(s: MicEncryptedSection<'a>) -> Self {
        IntermediateSection::Ciphertext(CiphertextSection::MicEncryptedIdentity(s))
    }
}

impl<'a> From<PlaintextSection<'a>> for IntermediateSection<'a> {
    fn from(s: PlaintextSection<'a>) -> Self {
        IntermediateSection::Plaintext(s)
    }
}
