/* This file is autogenerated. DO NOT CHANGE. All changes will be lost */

#include "qmlqtgrpcgen_client.grpc.qpb.h"

namespace qtgrpc::tests {
namespace TestService {
using namespace Qt::StringLiterals;

QmlClient::QmlClient(QObject *parent)
    : Client(parent)
{
}


void QmlClient::testMethod(const qtgrpc::tests::IntMessage &arg,
                const QJSValue &finishCallback,
                const QJSValue &errorCallback,
                const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)
{
    QJSEngine *jsEngine = qjsEngine(this);
    if (jsEngine == nullptr) {
        qWarning() << "Unable to call QmlClient::testMethod, it's only callable from JS engine context";
        return;
    }

    auto reply = call("testMethod"_L1, arg, options ? options->options() : QGrpcCallOptions{});
    QtGrpcQuickFunctional::makeCallConnections<qtgrpc::tests::IntMessage>(jsEngine,
                        std::move(reply), finishCallback, errorCallback);
}


void QmlClient::testMethodServerStream(const qtgrpc::tests::IntMessage &arg,
            const QJSValue &messageCallback,
            const QJSValue &finishCallback,
            const QJSValue &errorCallback,
            const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)
{
    QJSEngine *jsEngine = qjsEngine(this);
    if (jsEngine == nullptr) {
        qWarning() << "Unable to call QmlClient::testMethodServerStream, it's only callable from JS engine context";
        return;
    }

    auto stream = serverStream("testMethodServerStream"_L1, arg, options ? options->options() : QGrpcCallOptions{});
    QtGrpcQuickFunctional::makeServerStreamConnections<qtgrpc::tests::IntMessage>(jsEngine,
                        std::move(stream),
                        messageCallback, finishCallback, errorCallback);
}


TestMethodClientStreamSender *QmlClient::testMethodClientStream(const qtgrpc::tests::IntMessage &arg,
        const QJSValue &finishCallback,
        const QJSValue &errorCallback,
        const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)
{
    QJSEngine *jsEngine = qjsEngine(this);
    if (jsEngine == nullptr) {
        qWarning() << "Unable to call QmlClient::testMethodClientStream, it's only callable from JS engine context";
        return nullptr;
    }

    auto stream = clientStream("testMethodClientStream"_L1, arg, options ? options->options() : QGrpcCallOptions{});
    auto *sender = new TestMethodClientStreamSender(stream.get());
    QtGrpcQuickFunctional::makeClientStreamConnections<qtgrpc::tests::IntMessage>(jsEngine,
                        std::move(stream), finishCallback, errorCallback);
    QJSEngine::setObjectOwnership(sender, QJSEngine::JavaScriptOwnership);
    return sender;
}


TestMethodBiStreamSender *QmlClient::testMethodBiStream(const qtgrpc::tests::IntMessage &arg,
    const QJSValue &messageCallback,
    const QJSValue &finishCallback,
    const QJSValue &errorCallback,
    const QtGrpcQuickPrivate::QQmlGrpcCallOptions *options)
{
    QJSEngine *jsEngine = qjsEngine(this);
    if (jsEngine == nullptr) {
        qWarning() << "Unable to call QmlClient::testMethodBiStream, it's only callable from JS engine context";
        return nullptr;
    }

    auto stream = bidiStream("testMethodBiStream"_L1, arg, options ? options->options() : QGrpcCallOptions {});
    auto *sender = new TestMethodBiStreamSender(stream.get());
    QtGrpcQuickFunctional::makeBidiStreamConnections<qtgrpc::tests::IntMessage>(jsEngine,
                        std::move(stream), messageCallback, finishCallback, errorCallback);
    QJSEngine::setObjectOwnership(sender, QJSEngine::JavaScriptOwnership);
    return sender;
}

} // namespace TestService
} // namespace qtgrpc::tests

