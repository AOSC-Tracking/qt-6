// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickborderimage_p.h"
#include "qquickborderimage_p_p.h"

#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlengine.h>
#if QT_CONFIG(qml_network)
#include <QtNetwork/qnetworkreply.h>
#endif
#include <QtCore/qfile.h>
#include <QtCore/qmath.h>
#include <QtGui/qguiapplication.h>

#include <private/qqmlglobal_p.h>
#include <private/qsgadaptationlayer_p.h>

QT_BEGIN_NAMESPACE


/*!
    \qmltype BorderImage
    \nativetype QQuickBorderImage
    \inqmlmodule QtQuick
    \brief Paints a border based on an image.
    \inherits Item
    \ingroup qtquick-visual

    The BorderImage type is used to create borders out of images by scaling or tiling
    parts of each image.

    A BorderImage breaks a source image, specified using the \l source property,
    into 9 regions, as shown below:

    \image declarative-scalegrid.png

    When the image is scaled, regions of the source image are scaled or tiled to
    create the displayed border image in the following way:

    \list
    \li The corners (regions 1, 3, 7, and 9) are not scaled at all.
    \li Regions 2 and 8 are scaled according to
       \l{BorderImage::horizontalTileMode}{horizontalTileMode}.
    \li Regions 4 and 6 are scaled according to
       \l{BorderImage::verticalTileMode}{verticalTileMode}.
    \li The middle (region 5) is scaled according to both
       \l{BorderImage::horizontalTileMode}{horizontalTileMode} and
       \l{BorderImage::verticalTileMode}{verticalTileMode}.
    \endlist

    The regions of the image are defined using the \l border property group, which
    describes the distance from each edge of the source image to use as a border.

    \section1 Example Usage

    The following examples show the effects of the different modes on an image.
    Guide lines are overlaid onto the image to show the different regions of the
    image as described above.

    \beginfloatleft
    \image qml-borderimage-normal-image.png
    \endfloat

    For comparison, an unscaled image is displayed using a simple Image item.
    Here we have overlaid lines to show how we'd like to break it up with BorderImage:

    \snippet qml/borderimage/normal-image.qml normal image

    \clearfloat
    \beginfloatleft
    \image qml-borderimage-scaled.png
    \endfloat

    But when a BorderImage is used to display the image, the \l border property is
    used to determine the parts of the image that will lie inside the unscaled corner
    areas, and the parts that will be stretched horizontally and vertically.
    Then, you can give it a size that is
    larger than the original image. Since the \l horizontalTileMode property is set to
    \l{BorderImage::horizontalTileMode}{BorderImage.Stretch}, the parts of image in
    regions 2 and 8 are stretched horizontally. Since the \l verticalTileMode property
    is set to \l{BorderImage::verticalTileMode}{BorderImage.Stretch}, the parts of image
    in regions 4 and 6 are stretched vertically:

    \snippet qml/borderimage/borderimage-scaled.qml scaled border image

    \clearfloat
    \beginfloatleft
    \image qml-borderimage-tiled.png
    \endfloat

    Again, a large BorderImage is used to display the image. With the
    \l horizontalTileMode property set to \l{BorderImage::horizontalTileMode}{BorderImage.Repeat},
    the parts of image in regions 2 and 8 are tiled so that they fill the space at the
    top and bottom of the item. Similarly, the \l verticalTileMode property is set to
    \l{BorderImage::verticalTileMode}{BorderImage.Repeat}, so the parts of image in regions
    4 and 6 are tiled to fill the space at the left and right of the item:

    \snippet qml/borderimage/borderimage-tiled.qml tiled border image

    \clearfloat
    \beginfloatleft
    \image qml-borderimage-rounded.png
    \endfloat

    In some situations, the width of regions 2 and 8 may not be an exact multiple of the width
    of the corresponding regions in the source image. Similarly, the height of regions 4 and 6
    may not be an exact multiple of the height of the corresponding regions. If you use
    \l{BorderImage::horizontalTileMode}{BorderImage.Round} mode, it will choose an integer
    number of tiles and shrink them to fit:

    \snippet qml/borderimage/borderimage-rounded.qml tiled border image

    \clearfloat

    The Border Image example in \l{Qt Quick Examples - Image Elements} shows how a BorderImage
    can be used to simulate a shadow effect on a rectangular item.

    \section1 Image Loading

    The source image may not be loaded instantaneously, depending on its original location.
    Loading progress can be monitored with the \l progress property.

    \sa Image, AnimatedImage
 */

/*!
    \qmlproperty bool QtQuick::BorderImage::asynchronous

    Specifies that images on the local filesystem should be loaded
    asynchronously in a separate thread.  The default value is
    false, causing the user interface thread to block while the
    image is loaded.  Setting \a asynchronous to true is useful where
    maintaining a responsive user interface is more desirable
    than having images immediately visible.

    Note that this property is only valid for images read from the
    local filesystem.  Images loaded via a network resource (e.g. HTTP)
    are always loaded asynchronously.
*/
QQuickBorderImage::QQuickBorderImage(QQuickItem *parent)
: QQuickImageBase(*(new QQuickBorderImagePrivate), parent)
{
    connect(this, &QQuickImageBase::sourceSizeChanged, this, &QQuickBorderImage::sourceSizeChanged);
}

QQuickBorderImage::~QQuickBorderImage()
{
#if QT_CONFIG(qml_network)
    Q_D(QQuickBorderImage);
    if (d->sciReply)
        d->sciReply->deleteLater();
#endif
}

/*!
    \qmlproperty enumeration QtQuick::BorderImage::status

    This property describes the status of image loading.  It can be one of:

    \value BorderImage.Null     No image has been set
    \value BorderImage.Ready    The image has been loaded
    \value BorderImage.Loading  The image is currently being loaded
    \value BorderImage.Error    An error occurred while loading the image

    \sa progress
*/

/*!
    \qmlproperty real QtQuick::BorderImage::progress

    This property holds the progress of image loading, from 0.0 (nothing loaded)
    to 1.0 (finished).

    \sa status
*/

/*!
    \qmlproperty bool QtQuick::BorderImage::smooth

    This property holds whether the image is smoothly filtered when scaled or
    transformed.  Smooth filtering gives better visual quality, but it may be slower
    on some hardware.  If the image is displayed at its natural size, this property
    has no visual or performance effect.

    By default, this property is set to true.
*/

/*!
    \qmlproperty bool QtQuick::BorderImage::cache

    Specifies whether the image should be cached. The default value is
    true. Setting \a cache to false is useful when dealing with large images,
    to make sure that they aren't cached at the expense of small 'ui element' images.
*/

/*!
    \qmlproperty bool QtQuick::BorderImage::mirror

    This property holds whether the image should be horizontally inverted
    (effectively displaying a mirrored image).

    The default value is false.
*/

/*!
    \qmlproperty url QtQuick::BorderImage::source

    This property holds the URL that refers to the source image.

    BorderImage can handle any image format supported by Qt, loaded from any
    URL scheme supported by Qt.

    This property can also be used to refer to .sci files, which are
    written in a QML-specific, text-based format that specifies the
    borders, the image file and the tile rules for a given border image.

    The following .sci file sets the borders to 10 on each side for the
    image \c picture.png:

    \code
    border.left: 10
    border.top: 10
    border.bottom: 10
    border.right: 10
    source: "picture.png"
    \endcode

    The URL may be absolute, or relative to the URL of the component.

    \sa QQuickImageProvider
*/

/*!
    \qmlproperty size QtQuick::BorderImage::sourceSize

    This property holds the actual width and height of the loaded image.

    In BorderImage, this property is read-only.

    \sa Image::sourceSize
*/
void QQuickBorderImage::setSource(const QUrl &url)
{
    Q_D(QQuickBorderImage);

    if (url == d->url)
        return;

#if QT_CONFIG(qml_network)
    if (d->sciReply) {
        d->sciReply->deleteLater();
        d->sciReply = nullptr;
    }
#endif

    d->url = url;
    d->sciurl = QUrl();
    emit sourceChanged(d->url);

    if (isComponentComplete())
        load();
}

void QQuickBorderImage::load()
{
    Q_D(QQuickBorderImage);

    if (d->url.isEmpty()) {
        loadEmptyUrl();
    } else {
        if (d->url.path().endsWith(QLatin1String("sci"))) {
            const QQmlContext *context = qmlContext(this);
            QString lf = QQmlFile::urlToLocalFileOrQrc(context ? context->resolvedUrl(d->url)
                                                               : d->url);
            if (!lf.isEmpty()) {
                QFile file(lf);
                if (!file.open(QIODevice::ReadOnly))
                    d->setStatus(Error);
                else
                    setGridScaledImage(QQuickGridScaledImage(&file));
            } else {
#if QT_CONFIG(qml_network)
                d->setProgress(0);
                d->setStatus(Loading);

                QNetworkRequest req(d->url);
                d->sciReply = qmlEngine(this)->networkAccessManager()->get(req);
                qmlobject_connect(d->sciReply, QNetworkReply, SIGNAL(finished()),
                                  this, QQuickBorderImage, SLOT(sciRequestFinished()));
#endif
            }
        } else {
            loadPixmap(d->url, LoadPixmapOptions(HandleDPR | UseProviderOptions));
        }
    }
}

/*!
    \qmlpropertygroup QtQuick::BorderImage::border
    \qmlproperty int QtQuick::BorderImage::border.left
    \qmlproperty int QtQuick::BorderImage::border.right
    \qmlproperty int QtQuick::BorderImage::border.top
    \qmlproperty int QtQuick::BorderImage::border.bottom

    The 4 border lines (2 horizontal and 2 vertical) break the image into 9 sections,
    as shown below:

    \image declarative-scalegrid.png

    Each border line (left, right, top, and bottom) specifies an offset in pixels
    from the respective edge of the source image. By default, each border line has
    a value of 0.

    For example, the following definition sets the bottom line 10 pixels up from
    the bottom of the image:

    \qml
    BorderImage {
        border.bottom: 10
        // ...
    }
    \endqml

    The border lines can also be specified using a
    \l {BorderImage::source}{.sci file}.
*/

QQuickScaleGrid *QQuickBorderImage::border()
{
    Q_D(QQuickBorderImage);
    return d->getScaleGrid();
}

/*!
    \qmlproperty enumeration QtQuick::BorderImage::horizontalTileMode
    \qmlproperty enumeration QtQuick::BorderImage::verticalTileMode

    This property describes how to repeat or stretch the middle parts of the border image.

    \value BorderImage.Stretch  Scales the image to fit to the available area.
    \value BorderImage.Repeat   Tile the image until there is no more space. May crop the last image.
    \value BorderImage.Round    Like Repeat, but scales the images down to ensure that the last image is not cropped.

    The default tile mode for each property is BorderImage.Stretch.
*/
QQuickBorderImage::TileMode QQuickBorderImage::horizontalTileMode() const
{
    Q_D(const QQuickBorderImage);
    return d->horizontalTileMode;
}

void QQuickBorderImage::setHorizontalTileMode(TileMode t)
{
    Q_D(QQuickBorderImage);
    if (t != d->horizontalTileMode) {
        d->horizontalTileMode = t;
        emit horizontalTileModeChanged();
        update();
    }
}

QQuickBorderImage::TileMode QQuickBorderImage::verticalTileMode() const
{
    Q_D(const QQuickBorderImage);
    return d->verticalTileMode;
}

void QQuickBorderImage::setVerticalTileMode(TileMode t)
{
    Q_D(QQuickBorderImage);
    if (t != d->verticalTileMode) {
        d->verticalTileMode = t;
        emit verticalTileModeChanged();
        update();
    }
}

void QQuickBorderImage::setGridScaledImage(const QQuickGridScaledImage& sci)
{
    Q_D(QQuickBorderImage);
    if (!sci.isValid()) {
        d->setStatus(Error);
    } else {
        QQuickScaleGrid *sg = border();
        sg->setTop(sci.gridTop());
        sg->setBottom(sci.gridBottom());
        sg->setLeft(sci.gridLeft());
        sg->setRight(sci.gridRight());
        d->horizontalTileMode = sci.horizontalTileRule();
        d->verticalTileMode = sci.verticalTileRule();

        d->sciurl = d->url.resolved(QUrl(sci.pixmapUrl()));
        loadPixmap(d->sciurl);
    }
}

void QQuickBorderImage::requestFinished()
{
    Q_D(QQuickBorderImage);

    if (d->pendingPix != d->currentPix) {
        std::swap(d->pendingPix, d->currentPix);
        d->pendingPix->clear(this); // Clear the old image
    }

    const QSize impsize = d->currentPix->implicitSize();
    setImplicitSize(impsize.width() / d->devicePixelRatio, impsize.height() / d->devicePixelRatio);

    if (d->currentPix->isError()) {
        qmlWarning(this) << d->currentPix->error();
        d->setStatus(Error);
        d->setProgress(0);
    } else {
        d->setStatus(Ready);
        d->setProgress(1);
    }

    if (sourceSize() != d->oldSourceSize) {
        d->oldSourceSize = sourceSize();
        emit sourceSizeChanged();
    }
    if (d->frameCount != d->currentPix->frameCount()) {
        d->frameCount = d->currentPix->frameCount();
        emit frameCountChanged();
    }

    pixmapChange();
}

#if QT_CONFIG(qml_network)
void QQuickBorderImage::sciRequestFinished()
{
    Q_D(QQuickBorderImage);

    if (d->sciReply->error() != QNetworkReply::NoError) {
        d->setStatus(Error);
        d->sciReply->deleteLater();
        d->sciReply = nullptr;
    } else {
        QQuickGridScaledImage sci(d->sciReply);
        d->sciReply->deleteLater();
        d->sciReply = nullptr;
        setGridScaledImage(sci);
    }
}
#endif // qml_network

void QQuickBorderImage::doUpdate()
{
    update();
}

void QQuickBorderImagePrivate::calculateRects(const QQuickScaleGrid *border,
                                              const QSize &sourceSize,
                                              const QSizeF &targetSize,
                                              int horizontalTileMode,
                                              int verticalTileMode,
                                              qreal devicePixelRatio,
                                              QRectF *targetRect,
                                              QRectF *innerTargetRect,
                                              QRectF *innerSourceRect,
                                              QRectF *subSourceRect)
{
    *innerSourceRect = QRectF(0, 0, 1, 1);
    *targetRect = QRectF(0, 0, targetSize.width(), targetSize.height());
    *innerTargetRect = *targetRect;

    if (border) {
        qreal borderLeft = border->left() * devicePixelRatio;
        qreal borderRight = border->right() * devicePixelRatio;
        qreal borderTop = border->top() * devicePixelRatio;
        qreal borderBottom = border->bottom() * devicePixelRatio;
        if (borderLeft + borderRight > sourceSize.width() && borderLeft < sourceSize.width())
            borderRight = sourceSize.width() - borderLeft;
        if (borderTop + borderBottom > sourceSize.height() && borderTop < sourceSize.height())
            borderBottom = sourceSize.height() - borderTop;
        *innerSourceRect = QRectF(QPointF(borderLeft / qreal(sourceSize.width()),
                                          borderTop / qreal(sourceSize.height())),
                                  QPointF((sourceSize.width() - borderRight) / qreal(sourceSize.width()),
                                          (sourceSize.height() - borderBottom) / qreal(sourceSize.height()))),
        *innerTargetRect = QRectF(border->left(),
                                  border->top(),
                                  qMax<qreal>(0, targetSize.width() - (border->right() + border->left())),
                                  qMax<qreal>(0, targetSize.height() - (border->bottom() + border->top())));
    }

    qreal hTiles = 1;
    qreal vTiles = 1;
    const QSizeF innerTargetSize = innerTargetRect->size() * devicePixelRatio;
    if (innerSourceRect->width() <= 0)
        hTiles = 0;
    else if (horizontalTileMode != QQuickBorderImage::Stretch) {
        hTiles = innerTargetSize.width() / qreal(innerSourceRect->width() * sourceSize.width());
        if (horizontalTileMode == QQuickBorderImage::Round)
            hTiles = qCeil(hTiles);
    }
    if (innerSourceRect->height() <= 0)
        vTiles = 0;
    else if (verticalTileMode != QQuickBorderImage::Stretch) {
        vTiles = innerTargetSize.height() / qreal(innerSourceRect->height() * sourceSize.height());
        if (verticalTileMode == QQuickBorderImage::Round)
            vTiles = qCeil(vTiles);
    }

    *subSourceRect = QRectF(0, 0, hTiles, vTiles);
}


QSGNode *QQuickBorderImage::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    Q_D(QQuickBorderImage);

    QSGTexture *texture = d->sceneGraphRenderContext()->textureForFactory(d->currentPix->textureFactory(), window());

    if (!texture || width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    QSGInternalImageNode *node = static_cast<QSGInternalImageNode *>(oldNode);

    bool updatePixmap = d->pixmapChanged;
    d->pixmapChanged = false;
    if (!node) {
        node = d->sceneGraphContext()->createInternalImageNode(d->sceneGraphRenderContext());
        updatePixmap = true;
    }

    if (updatePixmap)
        node->setTexture(texture);

    // Don't implicitly create the scalegrid in the rendering thread...
    QRectF targetRect;
    QRectF innerTargetRect;
    QRectF innerSourceRect;
    QRectF subSourceRect;
    d->calculateRects(d->border,
                      QSize(d->currentPix->width(), d->currentPix->height()), QSizeF(width(), height()),
                      d->horizontalTileMode, d->verticalTileMode, d->devicePixelRatio,
                      &targetRect, &innerTargetRect,
                      &innerSourceRect, &subSourceRect);

    node->setTargetRect(targetRect);
    node->setInnerSourceRect(innerSourceRect);
    node->setInnerTargetRect(innerTargetRect);
    node->setSubSourceRect(subSourceRect);
    node->setMirror(d->mirrorHorizontally, d->mirrorVertically);

    node->setMipmapFiltering(QSGTexture::None);
    node->setFiltering(d->smooth ? QSGTexture::Linear : QSGTexture::Nearest);
    if (innerSourceRect == QRectF(0, 0, 1, 1) && (subSourceRect.width() > 1 || subSourceRect.height() > 1)) {
        node->setHorizontalWrapMode(QSGTexture::Repeat);
        node->setVerticalWrapMode(QSGTexture::Repeat);
    } else {
        node->setHorizontalWrapMode(QSGTexture::ClampToEdge);
        node->setVerticalWrapMode(QSGTexture::ClampToEdge);
    }
    node->setAntialiasing(d->antialiasing);
    node->update();

    return node;
}

void QQuickBorderImage::pixmapChange()
{
    Q_D(QQuickBorderImage);
    d->pixmapChanged = true;
    update();
}

/*!
    \qmlproperty int QtQuick::BorderImage::currentFrame
    \qmlproperty int QtQuick::BorderImage::frameCount
    \since 5.14

    currentFrame is the frame that is currently visible. The default is \c 0.
    You can set it to a number between \c 0 and \c {frameCount - 1} to display a
    different frame, if the image contains multiple frames.

    frameCount is the number of frames in the image. Most images have only one frame.
*/

/*!
    \qmlproperty bool QtQuick::BorderImage::retainWhileLoading
    \since 6.8

    \include qquickimage.cpp qml-image-retainwhileloading
 */

QT_END_NAMESPACE

#include "moc_qquickborderimage_p.cpp"
