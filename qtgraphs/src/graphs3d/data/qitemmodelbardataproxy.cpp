// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "baritemmodelhandler_p.h"
#include "qitemmodelbardataproxy_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QItemModelBarDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief Proxy class for presenting data in item models with Q3DBarsWidgetItem.
 *
 * QItemModelBarDataProxy allows you to use QAbstractItemModel derived models as
 * a data source for Q3DBarsWidgetItem. It uses the defined mappings to map data from the
 * model to rows, columns, and values of Q3DBarsWidgetItem graph.
 *
 * The data is resolved asynchronously whenever mappings or the model changes.
 * QBarDataProxy::arrayReset() is emitted when the data has been resolved.
 * However, when useModelCategories property is set to true, single item changes
 * are resolved synchronously, unless the same frame also contains a change that
 * causes the whole model to be resolved.
 *
 * Mappings can be used in the following ways:
 *
 * \list
 * \li If useModelCategories property is set to true, this proxy will map rows and
 *    columns of QAbstractItemModel directly to rows and columns of Q3DBarsWidgetItem, and uses the value
 *    returned for Qt::DisplayRole as bar value by default.
 *    The value role to be used can be redefined if Qt::DisplayRole is not suitable.
 *
 * \li For models that do not have data already neatly sorted into rows and columns, such as
 *    QAbstractListModel based models, you can define a role from the model to map for each of the
 *    row, column and value.
 *
 * \li If you do not want to include all data contained in the model, or the autogenerated rows and
 *    columns are not ordered as you wish, you can specify which rows and columns should be included
 *    and in which order by defining an explicit list of categories for either or both of rows and
 *    columns.
 * \endlist
 *
 * For example, assume that you have a custom QAbstractItemModel for storing various monthly values
 * related to a business.
 * Each item in the model has the roles "year", "month", "income", and "expenses".
 * You could do the following to display the data in a bar graph:
 *
 * \snippet doc_src_qtgraphs.cpp barmodelproxy
 *
 * If the fields of the model do not contain the data in the exact format you
 * need, you can specify a search pattern regular expression and a replace rule
 * for each role to get the value in a format you need. For more information on how
 * the replacement using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation. Note
 * that using regular expressions has an impact on the performance, so it's more
 * efficient to utilize item models where doing search and replace is not
 * necessary to get the desired values.
 *
 * For example about using the search patterns in conjunction with the roles,
 * see \l{Simple Bar Graph}.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmltype ItemModelBarDataProxy
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QItemModelBarDataProxy
 * \inherits BarDataProxy
 * \brief Proxy class for presenting data in item models with Bars3D.
 *
 * This type allows you to use AbstractItemModel derived models as a data source
 * for Bars3D.
 *
 * Data is resolved asynchronously whenever the mapping or the model changes.
 * QBarDataProxy::arrayReset() is emitted when the data has been resolved.
 *
 * For ItemModelBarDataProxy enums, see
 * \l{QItemModelBarDataProxy::MultiMatchBehavior}.
 *
 * For more details, see QItemModelBarDataProxy documentation.
 *
 * Usage example:
 *
 * \snippet doc_src_qmlgraphs.cpp 7
 *
 * \sa BarDataProxy, {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty model ItemModelBarDataProxy::itemModel
 * The item model.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rowRole
 * The item model role to map into row category.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::columnRole
 * The item model role to map into column category.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::valueRole
 * The item model role to map into bar value.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rotationRole
 * The item model role to map into bar rotation angle.
 */

/*!
 * \qmlproperty list<String> ItemModelBarDataProxy::rowCategories
 * The row categories of the mapping. Only items with row role values that are
 * found in this list are included when the data is resolved. The rows are
 * ordered in the same order as they are in this list.
 */

/*!
 * \qmlproperty list<String> ItemModelBarDataProxy::columnCategories
 * The column categories of the mapping. Only items with column role values that
 * are found in this list are included when the data is resolved. The columns
 * are ordered in the same order as they are in this list.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::useModelCategories
 * When set to \c true, the mapping ignores row and column roles and categories,
 * and uses the rows and columns from the model instead. Row and column headers
 * are used for row and column labels. Defaults to \c{false}.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::autoRowCategories
 * When set to \c true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from the model is resolved. Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::autoColumnCategories
 * When set to \c true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::rowRolePattern
 * When set, a search and replace is done on the value mapped by row role before
 * it is used as a row category. This property specifies the regular expression
 * to find the portion of the mapped value to replace and rowRoleReplace
 * property contains the replacement string. This is useful for example in
 * parsing row and column categories from a single timestamp field in the item
 * model.
 *
 * \sa rowRole, rowRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::columnRolePattern
 * When set, a search and replace is done on the value mapped by column role
 * before it is used as a column category. This property specifies the regular
 * expression to find the portion of the mapped value to replace and
 * columnRoleReplace property contains the replacement string. This is useful
 * for example in parsing row and column categories from a single timestamp
 * field in the item model.
 *
 * \sa columnRole, columnRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::valueRolePattern
 * When set, a search and replace is done on the value mapped by value role
 * before it is used as a bar value. This property specifies the regular
 * expression to find the portion of the mapped value to replace and
 * valueRoleReplace property contains the replacement string.
 *
 * \sa valueRole, valueRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::rotationRolePattern
 * When set, a search and replace is done on the value mapped by rotation role
 * before it is used as a bar rotation angle. This property specifies the
 * regular expression to find the portion of the mapped value to replace and
 * rotationRoleReplace property contains the replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rowRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * rowRolePattern. Defaults to empty string. For more information on how the
 * search and replace using regular expressions works, see
 * QString::replace(const QRegularExpression &rx, const QString &after) function
 * documentation.
 *
 * \sa rowRole, rowRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::columnRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * columnRolePattern. Defaults to empty string. For more information on how the
 * search and replace using regular expressions works, see
 * QString::replace(const QRegularExpression &rx, const QString &after) function
 * documentation.
 *
 * \sa columnRole, columnRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::valueRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * valueRolePattern. Defaults to empty string. For more information on how the
 * search and replace using regular expressions works, see
 * QString::replace(const QRegularExpression &rx, const QString &after) function
 * documentation.
 *
 * \sa valueRole, valueRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rotationRoleReplace
 * This property defines the replacement content to be used in conjunction with
 * rotationRolePattern. Defaults to empty string. For more information on how
 * the search and replace using regular expressions works, see
 * QString::replace(const QRegularExpression &rx, const QString &after) function
 * documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */

/*!
 * \qmlproperty enumeration ItemModelBarDataProxy::multiMatchBehavior
 * Defines how multiple matches for each row/column combination are handled.
 * Defaults to
 * \l{QItemModelBarDataProxy::MultiMatchBehavior::Last}
 *   {ItemModelBarDataProxy.MultiMatchBehavior.Last}.
 * The chosen behavior affects both bar value and rotation.
 *
 * For example, you might have an item model with timestamped data taken at
 * irregular intervals and you want to visualize total value of data items on
 * each day with a bar graph. This can be done by specifying row and column
 * categories so that each bar represents a day, and setting multiMatchBehavior
 * to
 * \l{QItemModelBarDataProxy::MultiMatchBehavior::Cumulative}
 * {ItemModelBarDataProxy.MultiMatchBehavior.Cumulative}.
 */

/*!
    \qmlsignal ItemModelBarDataProxy::itemModelChanged(model itemModel)

    This signal is emitted when itemModel changes to \a itemModel.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rowRoleChanged(string role)

    This signal is emitted when rowRole changes to \a role.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::columnRoleChanged(string role)

    This signal is emitted when columnRole changes to \a role.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::valueRoleChanged(string role)

    This signal is emitted when valueRole changes to \a role.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rotationRoleChanged(string role)

    This signal is emitted when rotationRole changes to \a role.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rowCategoriesChanged()

    This signal is emitted when rowCategories changes.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::columnCategoriesChanged()

    This signal is emitted when columnCategories changes.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::useModelCategoriesChanged(bool enable)

    This signal is emitted when useModelCategories changes to \a enable.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::autoRowCategoriesChanged(bool enable)

    This signal is emitted when autoRowCategories changes to \a enable.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::autoColumnCategoriesChanged(bool enable)

    This signal is emitted when autoColumnCategories changes to \a enable.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rowRolePatternChanged(regExp pattern)

    This signal is emitted when rowRolePattern changes to \a pattern.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::columnRolePatternChanged(regExp pattern)

    This signal is emitted when columnRolePattern changes to \a pattern.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::valueRolePatternChanged(regExp pattern)

    This signal is emitted when valueRolePattern changes to \a pattern.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rotationRolePatternChanged(regExp pattern)

    This signal is emitted when rotationRolePattern changes to \a pattern.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rowRoleReplaceChanged(string replace)

    This signal is emitted when rowRoleReplace changes to \a replace.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::columnRoleReplaceChanged(string replace)

    This signal is emitted when columnRoleReplace changes to \a replace.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::valueRoleReplaceChanged(string replace)

    This signal is emitted when valueRoleReplace changes to \a replace.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::rotationRoleReplaceChanged(string replace)

    This signal is emitted when rotationRoleReplace changes to \a replace.
*/
/*!
    \qmlsignal ItemModelBarDataProxy::multiMatchBehaviorChanged(enumeration behavior)

    This signal is emitted when multiMatchBehavior changes to \a behavior.
*/

/*!
 *  \enum QItemModelBarDataProxy::MultiMatchBehavior
 *
 *  Behavior types for QItemModelBarDataProxy::multiMatchBehavior property.
 *
 *  \value First
 *         The value is taken from the first item in the item model that matches
 *         each row/column combination.
 *  \value Last
 *         The value is taken from the last item in the item model that matches
 *         each row/column combination.
 *  \value Average
 *         The values from all items matching each row/column combination are
 *         averaged together and the average is used as the bar value.
 *  \value Cumulative
 *         The values from all items matching each row/column combination are
 *         added together and the total is used as the bar value.
 */

/*!
 * Constructs QItemModelBarDataProxy with an optional \a parent.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel, QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    setItemModel(itemModel);
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls. The value role is set to \a valueRole. This
 * constructor is meant to be used with models that have data properly sorted in
 * rows and columns already, so it also sets useModelCategories property to
 * true.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &valueRole,
                                               QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_valueRole = valueRole;
    d->m_useModelCategories = true;
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls. The role mappings are set with \a rowRole, \a
 * columnRole, and \a valueRole.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_rowRole = rowRole;
    d->m_columnRole = columnRole;
    d->m_valueRole = valueRole;
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls. The role mappings are set with \a rowRole, \a
 * columnRole, \a valueRole, and \a rotationRole.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QString &rotationRole,
                                               QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_rowRole = rowRole;
    d->m_columnRole = columnRole;
    d->m_valueRole = valueRole;
    d->m_rotationRole = rotationRole;
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls. The role mappings are set with \a rowRole, \a
 * columnRole, and \a valueRole. Row and column categories are set with \a
 * rowCategories and \a columnCategories. This constructor also sets
 * autoRowCategories and autoColumnCategories to false.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QStringList &rowCategories,
                                               const QStringList &columnCategories,
                                               QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_rowRole = rowRole;
    d->m_columnRole = columnRole;
    d->m_valueRole = valueRole;
    d->m_rowCategories = rowCategories;
    d->m_columnCategories = columnCategories;
    d->m_autoRowCategories = false;
    d->m_autoColumnCategories = false;
    d->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and an optional \a parent.
 * The proxy doesn't take ownership of the \a itemModel, as typically item models
 * are owned by other controls. The role mappings are set with \a rowRole, \a
 * columnRole, \a valueRole, and \a rotationRole. Row and column categories are
 * set with \a rowCategories and \a columnCategories. This constructor also sets
 * autoRowCategories and autoColumnCategories to false.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QString &rotationRole,
                                               const QStringList &rowCategories,
                                               const QStringList &columnCategories,
                                               QObject *parent)
    : QBarDataProxy(*(new QItemModelBarDataProxyPrivate(this)), parent)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
    d->m_rowRole = rowRole;
    d->m_columnRole = columnRole;
    d->m_valueRole = valueRole;
    d->m_rotationRole = rotationRole;
    d->m_rowCategories = rowCategories;
    d->m_columnCategories = columnCategories;
    d->m_autoRowCategories = false;
    d->m_autoColumnCategories = false;
    d->connectItemModelHandler();
}

/*!
 * Destroys QItemModelBarDataProxy.
 */
QItemModelBarDataProxy::~QItemModelBarDataProxy() {}

/*!
 * \property QItemModelBarDataProxy::itemModel
 *
 * \brief The item model.
 */

/*!
 * Sets the item model to \a itemModel. Does not take ownership of the model,
 * but does connect to it to listen for changes.
 */
void QItemModelBarDataProxy::setItemModel(QAbstractItemModel *itemModel)
{
    Q_D(QItemModelBarDataProxy);
    d->m_itemModelHandler->setItemModel(itemModel);
}

QAbstractItemModel *QItemModelBarDataProxy::itemModel() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_itemModelHandler->itemModel();
}

/*!
 * \property QItemModelBarDataProxy::rowRole
 *
 * \brief The row role for the mapping.
 */
void QItemModelBarDataProxy::setRowRole(const QString &role)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rowRole != role) {
        d->m_rowRole = role;
        emit rowRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::rowRole() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rowRole;
}

/*!
 * \property QItemModelBarDataProxy::columnRole
 *
 * \brief The column role for the mapping.
 */
void QItemModelBarDataProxy::setColumnRole(const QString &role)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_columnRole != role) {
        d->m_columnRole = role;
        emit columnRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::columnRole() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_columnRole;
}

/*!
 * \property QItemModelBarDataProxy::valueRole
 *
 * \brief The value role for the mapping.
 */
void QItemModelBarDataProxy::setValueRole(const QString &role)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_valueRole != role) {
        d->m_valueRole = role;
        emit valueRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::valueRole() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_valueRole;
}

/*!
 * \property QItemModelBarDataProxy::rotationRole
 *
 * \brief The rotation role for the mapping.
 */
void QItemModelBarDataProxy::setRotationRole(const QString &role)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rotationRole != role) {
        d->m_rotationRole = role;
        emit rotationRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::rotationRole() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rotationRole;
}

/*!
 * \property QItemModelBarDataProxy::rowCategories
 *
 * \brief The row categories for the mapping.
 */
void QItemModelBarDataProxy::setRowCategories(const QStringList &categories)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rowCategories != categories) {
        d->m_rowCategories = categories;
        emit rowCategoriesChanged();
    }
}

QStringList QItemModelBarDataProxy::rowCategories() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rowCategories;
}

/*!
 * \property QItemModelBarDataProxy::columnCategories
 *
 * \brief The column categories for the mapping.
 */
void QItemModelBarDataProxy::setColumnCategories(const QStringList &categories)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_columnCategories != categories) {
        d->m_columnCategories = categories;
        emit columnCategoriesChanged();
    }
}

QStringList QItemModelBarDataProxy::columnCategories() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_columnCategories;
}

/*!
 * \property QItemModelBarDataProxy::useModelCategories
 *
 * \brief Whether row and column roles and categories are used for mapping.
 *
 * When set to \c true, the mapping ignores row and column roles and categories,
 * and uses the rows and columns from the model instead. Defaults to \c{false}.
 */
void QItemModelBarDataProxy::setUseModelCategories(bool enable)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_useModelCategories != enable) {
        d->m_useModelCategories = enable;
        emit useModelCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::useModelCategories() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_useModelCategories;
}

/*!
 * \property QItemModelBarDataProxy::autoRowCategories
 *
 * \brief Whether row categories are generated automatically.
 *
 * When set to \c true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */
void QItemModelBarDataProxy::setAutoRowCategories(bool enable)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_autoRowCategories != enable) {
        d->m_autoRowCategories = enable;
        emit autoRowCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::autoRowCategories() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_autoRowCategories;
}

/*!
 * \property QItemModelBarDataProxy::autoColumnCategories
 *
 * \brief Whether column categories are generated automatically.
 *
 * When set to \c true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */
void QItemModelBarDataProxy::setAutoColumnCategories(bool enable)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_autoColumnCategories != enable) {
        d->m_autoColumnCategories = enable;
        emit autoColumnCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::autoColumnCategories() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_autoColumnCategories;
}

/*!
 * Changes \a rowRole, \a columnRole, \a valueRole, \a rotationRole,
 * \a rowCategories and \a columnCategories to the mapping.
 */
void QItemModelBarDataProxy::remap(const QString &rowRole,
                                   const QString &columnRole,
                                   const QString &valueRole,
                                   const QString &rotationRole,
                                   const QStringList &rowCategories,
                                   const QStringList &columnCategories)
{
    setRowRole(rowRole);
    setColumnRole(columnRole);
    setValueRole(valueRole);
    setRotationRole(rotationRole);
    setRowCategories(rowCategories);
    setColumnCategories(columnCategories);
}

/*!
 * Returns the index of the specified \a category in row categories list.
 * If the row categories list is empty, -1 is returned.
 * \note If the automatic row categories generation is in use, this method will
 * not return a valid index before the data in the model is resolved for the
 * first time.
 */
qsizetype QItemModelBarDataProxy::rowCategoryIndex(const QString &category)
{
    Q_D(QItemModelBarDataProxy);
    return d->m_rowCategories.indexOf(category);
}

/*!
 * Returns the index of the specified \a category in column categories list.
 * If the category is not found, -1 is returned.
 * \note If the automatic column categories generation is in use, this method
 * will not return a valid index before the data in the model is resolved for
 * the first time.
 */
qsizetype QItemModelBarDataProxy::columnCategoryIndex(const QString &category)
{
    Q_D(QItemModelBarDataProxy);
    return d->m_columnCategories.indexOf(category);
}

/*!
 * \property QItemModelBarDataProxy::rowRolePattern
 *
 * \brief Whether a search and replace is performed on the value mapped by row
 *  role before it is used as a row category.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and rowRoleReplace property contains the replacement
 * string. This is useful for example in parsing row and column categories from
 * a single timestamp field in the item model.
 *
 * \sa rowRole, rowRoleReplace
 */
void QItemModelBarDataProxy::setRowRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rowRolePattern != pattern) {
        d->m_rowRolePattern = pattern;
        emit rowRolePatternChanged(pattern);
    }
}

QRegularExpression QItemModelBarDataProxy::rowRolePattern() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rowRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::columnRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by column
 * role before it is used as a column category.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and columnRoleReplace property contains the
 * replacement string. This is useful for example in parsing row and column
 * categories from a single timestamp field in the item model.
 *
 * \sa columnRole, columnRoleReplace
 */
void QItemModelBarDataProxy::setColumnRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_columnRolePattern != pattern) {
        d->m_columnRolePattern = pattern;
        emit columnRolePatternChanged(pattern);
    }
}

QRegularExpression QItemModelBarDataProxy::columnRolePattern() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_columnRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::valueRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by value role
 * before it is used as a bar value.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and valueRoleReplace property contains the
 * replacement string.
 *
 * \sa valueRole, valueRoleReplace
 */
void QItemModelBarDataProxy::setValueRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_valueRolePattern != pattern) {
        d->m_valueRolePattern = pattern;
        emit valueRolePatternChanged(pattern);
    }
}

QRegularExpression QItemModelBarDataProxy::valueRolePattern() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_valueRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::rotationRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by rotation
 * role before it is used as a bar rotation angle.
 *
 * This property specifies the regular expression to find the portion
 * of the mapped value to replace and rotationRoleReplace property contains the
 * replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */
void QItemModelBarDataProxy::setRotationRolePattern(const QRegularExpression &pattern)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rotationRolePattern != pattern) {
        d->m_rotationRolePattern = pattern;
        emit rotationRolePatternChanged(pattern);
    }
}

QRegularExpression QItemModelBarDataProxy::rotationRolePattern() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rotationRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::rowRoleReplace
 *
 * \brief The replacement content to be used in conjunction with rowRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace
 * using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa rowRole, rowRolePattern
 */
void QItemModelBarDataProxy::setRowRoleReplace(const QString &replace)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rowRoleReplace != replace) {
        d->m_rowRoleReplace = replace;
        emit rowRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::rowRoleReplace() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rowRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::columnRoleReplace
 *
 * \brief The replacement content to be used in conjunction with columnRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace
 * using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa columnRole, columnRolePattern
 */
void QItemModelBarDataProxy::setColumnRoleReplace(const QString &replace)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_columnRoleReplace != replace) {
        d->m_columnRoleReplace = replace;
        emit columnRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::columnRoleReplace() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_columnRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::valueRoleReplace
 *
 * \brief The replacement content to be used in conjunction with valueRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace
 * using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa valueRole, valueRolePattern
 */
void QItemModelBarDataProxy::setValueRoleReplace(const QString &replace)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_valueRoleReplace != replace) {
        d->m_valueRoleReplace = replace;
        emit valueRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::valueRoleReplace() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_valueRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::rotationRoleReplace
 *
 * \brief The replacement content to be used in conjunction with
 * rotationRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace
 * using regular expressions works, see QString::replace(const
 * QRegularExpression &rx, const QString &after) function documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */
void QItemModelBarDataProxy::setRotationRoleReplace(const QString &replace)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_rotationRoleReplace != replace) {
        d->m_rotationRoleReplace = replace;
        emit rotationRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::rotationRoleReplace() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_rotationRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::multiMatchBehavior
 *
 * \brief How multiple matches for each row/column combination are handled.
 *
 * Defaults to QItemModelBarDataProxy::MultiMatchBehavior::Last. The chosen
 * behavior affects both bar value and rotation.
 *
 * For example, you might have an item model with timestamped data taken at
 * irregular intervals and you want to visualize total value of data items on
 * each day with a bar graph. This can be done by specifying row and column
 * categories so that each bar represents a day, and setting multiMatchBehavior
 * to QItemModelBarDataProxy::MultiMatchBehavior::Cumulative.
 */
void QItemModelBarDataProxy::setMultiMatchBehavior(
    QItemModelBarDataProxy::MultiMatchBehavior behavior)
{
    Q_D(QItemModelBarDataProxy);
    if (d->m_multiMatchBehavior != behavior) {
        d->m_multiMatchBehavior = behavior;
        emit multiMatchBehaviorChanged(behavior);
    }
}

QItemModelBarDataProxy::MultiMatchBehavior QItemModelBarDataProxy::multiMatchBehavior() const
{
    Q_D(const QItemModelBarDataProxy);
    return d->m_multiMatchBehavior;
}

// QItemModelBarDataProxyPrivate

QItemModelBarDataProxyPrivate::QItemModelBarDataProxyPrivate(QItemModelBarDataProxy *q)
    : m_itemModelHandler(new BarItemModelHandler(q))
    , m_useModelCategories(false)
    , m_autoRowCategories(true)
    , m_autoColumnCategories(true)
    , m_multiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior::Last)
{}

QItemModelBarDataProxyPrivate::~QItemModelBarDataProxyPrivate()
{
    delete m_itemModelHandler;
}

void QItemModelBarDataProxyPrivate::connectItemModelHandler()
{
    Q_Q(QItemModelBarDataProxy);
    QObject::connect(m_itemModelHandler,
                     &BarItemModelHandler::itemModelChanged,
                     q,
                     &QItemModelBarDataProxy::itemModelChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rowRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::columnRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::valueRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rotationRoleChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rowCategoriesChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::columnCategoriesChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::useModelCategoriesChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::autoRowCategoriesChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::autoColumnCategoriesChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rowRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::columnRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::valueRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rotationRolePatternChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rowRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::columnRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::valueRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::rotationRoleReplaceChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(q,
                     &QItemModelBarDataProxy::multiMatchBehaviorChanged,
                     m_itemModelHandler,
                     &AbstractItemModelHandler::handleMappingChanged);
}

QT_END_NAMESPACE
