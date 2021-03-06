/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "symboleditorstate_drawellipse.h"
#include <librepcb/common/graphics/graphicsview.h>
#include <librepcb/common/graphics/graphicsscene.h>
#include <librepcb/common/graphics/graphicslayer.h>
#include <librepcb/common/geometry/ellipse.h>
#include <librepcb/common/geometry/cmd/cmdellipseedit.h>
#include <librepcb/common/widgets/graphicslayercombobox.h>
#include <librepcb/library/sym/symbol.h>
#include <librepcb/library/sym/symbolgraphicsitem.h>
#include <librepcb/common/graphics/ellipsegraphicsitem.h>
#include "../symboleditorwidget.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace library {
namespace editor {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

SymbolEditorState_DrawEllipse::SymbolEditorState_DrawEllipse(const Context& context) noexcept :
    SymbolEditorState(context), mCurrentEllipse(nullptr), mCurrentGraphicsItem(nullptr),
    mLastLayerName(GraphicsLayer::sSymbolOutlines), mLastLineWidth(250000), mLastFill(false),
    mLastGrabArea(true)
{
}

SymbolEditorState_DrawEllipse::~SymbolEditorState_DrawEllipse() noexcept
{
    Q_ASSERT(mEditCmd.isNull());
    Q_ASSERT(mCurrentEllipse == nullptr);
    Q_ASSERT(mCurrentGraphicsItem == nullptr);
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

bool SymbolEditorState_DrawEllipse::entry() noexcept
{
    mContext.graphicsScene.setSelectionArea(QPainterPath()); // clear selection
    mContext.graphicsView.setCursor(Qt::CrossCursor);

    // populate command toolbar
    mContext.commandToolBar.addLabel(tr("Layer:"));
    std::unique_ptr<GraphicsLayerComboBox> layerComboBox(new GraphicsLayerComboBox());
    layerComboBox->setLayers(mContext.layerProvider.getSchematicGeometryElementLayers());
    layerComboBox->setCurrentLayer(mLastLayerName);
    connect(layerComboBox.get(), &GraphicsLayerComboBox::currentLayerChanged,
            this, &SymbolEditorState_DrawEllipse::layerComboBoxValueChanged);
    mContext.commandToolBar.addWidget(std::move(layerComboBox));

    mContext.commandToolBar.addLabel(tr("Line Width:"), 10);
    std::unique_ptr<QDoubleSpinBox> lineWidthSpinBox(new QDoubleSpinBox());
    lineWidthSpinBox->setMinimum(0);
    lineWidthSpinBox->setMaximum(100);
    lineWidthSpinBox->setSingleStep(0.1);
    lineWidthSpinBox->setDecimals(6);
    lineWidthSpinBox->setValue(mLastLineWidth.toMm());
    connect(lineWidthSpinBox.get(),
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &SymbolEditorState_DrawEllipse::lineWidthSpinBoxValueChanged);
    mContext.commandToolBar.addWidget(std::move(lineWidthSpinBox));

    std::unique_ptr<QCheckBox> fillCheckBox(new QCheckBox(tr("Fill")));
    fillCheckBox->setChecked(mLastFill);
    connect(fillCheckBox.get(), &QCheckBox::toggled,
            this, &SymbolEditorState_DrawEllipse::fillCheckBoxCheckedChanged);
    mContext.commandToolBar.addWidget(std::move(fillCheckBox));

    std::unique_ptr<QCheckBox> grabAreaCheckBox(new QCheckBox(tr("Grab Area")));
    grabAreaCheckBox->setChecked(mLastGrabArea);
    connect(grabAreaCheckBox.get(), &QCheckBox::toggled,
            this, &SymbolEditorState_DrawEllipse::grabAreaCheckBoxCheckedChanged);
    mContext.commandToolBar.addWidget(std::move(grabAreaCheckBox));

    return true;
}

bool SymbolEditorState_DrawEllipse::exit() noexcept
{
    if (mCurrentEllipse && (!abortAddEllipse())) {
        return false;
    }

    // cleanup command toolbar
    mContext.commandToolBar.clear();

    mContext.graphicsView.setCursor(Qt::ArrowCursor);
    return true;
}

/*****************************************************************************************
 *  Event Handlers
 ****************************************************************************************/

bool SymbolEditorState_DrawEllipse::processGraphicsSceneMouseMoved(QGraphicsSceneMouseEvent& e) noexcept
{
    if (mCurrentEllipse) {
        Point currentPos = Point::fromPx(e.scenePos(), getGridInterval());
        return updateEllipseSize(currentPos);
    } else {
        return true;
    }
}

bool SymbolEditorState_DrawEllipse::processGraphicsSceneLeftMouseButtonPressed(QGraphicsSceneMouseEvent& e) noexcept
{
    Point currentPos = Point::fromPx(e.scenePos(), getGridInterval());
    if (mCurrentEllipse) {
        return finishAddEllipse(currentPos);
    } else {
        return startAddEllipse(currentPos);
    }
}

bool SymbolEditorState_DrawEllipse::processAbortCommand() noexcept
{
    if (mCurrentEllipse) {
        return abortAddEllipse();
    } else {
        return false;
    }
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool SymbolEditorState_DrawEllipse::startAddEllipse(const Point& pos) noexcept
{
    try {
        mStartPos = pos;
        mContext.undoStack.beginCmdGroup(tr("Add symbol ellipse"));
        mCurrentEllipse = new Ellipse(Uuid::createRandom(), mLastLayerName, mLastLineWidth,
                                      mLastFill, mLastGrabArea, pos, Length(0), Length(0),
                                      Angle::deg0());
        mContext.undoStack.appendToCmdGroup(new CmdEllipseInsert(
            mContext.symbol.getEllipses(), std::shared_ptr<Ellipse>(mCurrentEllipse)));
        mEditCmd.reset(new CmdEllipseEdit(*mCurrentEllipse));
        mCurrentGraphicsItem = mContext.symbolGraphicsItem.getEllipseGraphicsItem(*mCurrentEllipse);
        Q_ASSERT(mCurrentGraphicsItem);
        mCurrentGraphicsItem->setSelected(true);
        return true;
    } catch (const Exception& e) {
        QMessageBox::critical(&mContext.editorWidget, tr("Error"), e.getMsg());
        mCurrentGraphicsItem = nullptr;
        mCurrentEllipse = nullptr;
        mEditCmd.reset();
        return false;
    }
}

bool SymbolEditorState_DrawEllipse::updateEllipseSize(const Point& pos) noexcept
{
    Point center = (pos + mStartPos) / 2;
    mEditCmd->setCenter(center, true);
    mEditCmd->setRadiusX((pos.getX() - mStartPos.getX()).abs() / 2, true);
    mEditCmd->setRadiusY((pos.getY() - mStartPos.getY()).abs() / 2, true);
    return true;
}

bool SymbolEditorState_DrawEllipse::finishAddEllipse(const Point& pos) noexcept
{
    if (pos == mStartPos) {
        return abortAddEllipse();
    }

    try {
        updateEllipseSize(pos);
        mCurrentGraphicsItem->setSelected(false);
        mCurrentGraphicsItem = nullptr;
        mCurrentEllipse = nullptr;
        mContext.undoStack.appendToCmdGroup(mEditCmd.take());
        mContext.undoStack.commitCmdGroup();
        return true;
    } catch (const Exception& e) {
        QMessageBox::critical(&mContext.editorWidget, tr("Error"), e.getMsg());
        return false;
    }
}

bool SymbolEditorState_DrawEllipse::abortAddEllipse() noexcept
{
    try {
        mCurrentGraphicsItem->setSelected(false);
        mCurrentGraphicsItem = nullptr;
        mCurrentEllipse = nullptr;
        mEditCmd.reset();
        mContext.undoStack.abortCmdGroup();
        return true;
    } catch (const Exception& e) {
        QMessageBox::critical(&mContext.editorWidget, tr("Error"), e.getMsg());
        return false;
    }
}

void SymbolEditorState_DrawEllipse::layerComboBoxValueChanged(const QString& layerName) noexcept
{
    if (layerName.isEmpty()) {
        return;
    }
    mLastLayerName = layerName;
    if (mEditCmd) {
        mEditCmd->setLayerName(mLastLayerName, true);
    }
}

void SymbolEditorState_DrawEllipse::lineWidthSpinBoxValueChanged(double value) noexcept
{
    mLastLineWidth = Length::fromMm(value);
    if (mEditCmd) {
        mEditCmd->setLineWidth(mLastLineWidth, true);
    }
}

void SymbolEditorState_DrawEllipse::fillCheckBoxCheckedChanged(bool checked) noexcept
{
    mLastFill = checked;
    if (mEditCmd) {
        mEditCmd->setIsFilled(mLastFill, true);
    }
}

void SymbolEditorState_DrawEllipse::grabAreaCheckBoxCheckedChanged(bool checked) noexcept
{
    mLastGrabArea = checked;
    if (mEditCmd) {
        mEditCmd->setIsGrabArea(mLastGrabArea, true);
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace editor
} // namespace library
} // namespace librepcb
