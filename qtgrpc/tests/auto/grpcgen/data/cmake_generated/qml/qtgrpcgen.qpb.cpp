/* This file is autogenerated. DO NOT CHANGE. All changes will be lost */


#include "qtgrpcgen.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

#include <cmath>

namespace qtgrpc::tests {

class IntMessage_QtProtobufData : public QSharedData
{
public:
    IntMessage_QtProtobufData()
        : QSharedData(),
          m_field(0)
    {
    }

    IntMessage_QtProtobufData(const IntMessage_QtProtobufData &other)
        : QSharedData(other),
          m_field(other.m_field)
    {
    }

    QtProtobuf::sint32 m_field;
};

IntMessage::~IntMessage() = default;

static constexpr struct {
    QtProtobufPrivate::QProtobufPropertyOrdering::Data data;
    const std::array<uint, 5> qt_protobuf_IntMessage_uint_data;
    const char qt_protobuf_IntMessage_char_data[31];
} qt_protobuf_IntMessage_metadata {
    // data
    {
        0, /* = version */
        1, /* = num fields */
        2, /* = field number offset */
        3, /* = property index offset */
        4, /* = field flags offset */
        23, /* = message full name length */
    },
    // uint_data
    {
        // JSON name offsets:
        24, /* = field */
        30, /* = end-of-string-marker */
        // Field numbers:
        1, /* = field */
        // Property indices:
        0, /* = field */
        // Field flags:
        uint(QtProtobufPrivate::FieldFlag::NoFlags), /* = field */
    },
    // char_data
    /* metadata char_data: */
    "qtgrpc.tests.IntMessage\0" /* = full message name */
    /* field char_data: */
    "field\0"
};

const QtProtobufPrivate::QProtobufPropertyOrdering IntMessage::staticPropertyOrdering = {
    &qt_protobuf_IntMessage_metadata.data
};

void IntMessage::registerTypes()
{
    qRegisterMetaType<IntMessage>();
    qRegisterMetaType<QList<IntMessage>>();
}

IntMessage::IntMessage()
    : QProtobufMessage(&IntMessage::staticMetaObject, &IntMessage::staticPropertyOrdering),
      dptr(new IntMessage_QtProtobufData)
{
}

IntMessage::IntMessage(const IntMessage &other)
    = default;
IntMessage &IntMessage::operator =(const IntMessage &other)
{
    IntMessage temp(other);
    swap(temp);
    return *this;
}
IntMessage::IntMessage(IntMessage &&other) noexcept
    = default;
bool comparesEqual(const IntMessage &lhs, const IntMessage &rhs) noexcept
{
    return operator ==(static_cast<const QProtobufMessage&>(lhs),
                       static_cast<const QProtobufMessage&>(rhs))
        && lhs.dptr->m_field == rhs.dptr->m_field;
}

QtProtobuf::sint32 IntMessage::field() const
{
    return dptr->m_field;
}

void IntMessage::setField(QtProtobuf::sint32 field)
{
    if (dptr->m_field != field) {
        dptr.detach();
        dptr->m_field = field;
    }
}

} // namespace qtgrpc::tests

#include "moc_qtgrpcgen.qpb.cpp"
