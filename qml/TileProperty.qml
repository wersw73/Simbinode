/*
 * Copyright © 2020 Gukova Anastasiia
 * Copyright © 2020 Gukov Anton <fexcron@gmail.com>
 *
 *
 * This file is part of Symbinode.
 *
 * Symbinode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Symbinode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symbinode.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.5

Item {
    width: parent.width
    height: childrenRect.height + 30
    property alias startOffsetX: offsetXParam.propertyValue
    property alias startOffsetY: offsetYParam.propertyValue
    property alias startColumns: columnsParam.propertyValue
    property alias startRows: rowsParam.propertyValue
    property alias startScaleX: scaleXParam.propertyValue
    property alias startScaleY: scaleYParam.propertyValue
    property alias startRotation: rotationParam.propertyValue
    property alias startRandPosition: randPositionParam.propertyValue
    property alias startRandRotation: randRotationParam.propertyValue
    property alias startRandScale: randScaleParam.propertyValue
    property alias startMask: maskStrengthParam.propertyValue
    property alias startInputsCount: inputsCountParam.propertyValue
    property alias startSeed: seedParam.propertyValue
    property alias startTileScale: scaleParam.propertyValue
    property alias startKeepProportion: keepProportionParam.checked
    property alias startUseAlpha: useAlphaParam.checked
    property alias startDepthMask: depthMaskParam.checked
    property alias startBits: bitsParam.currentIndex
    signal offsetXChanged(real offset)
    signal offsetYChanged(real offset)
    signal columnsChanged(int columns)
    signal rowsChanged(int rows)    
    signal scaleXChanged(real scale)
    signal scaleYChanged(real scale)
    signal rotationAngleChanged(int angle)
    signal randPositionChanged(real rand)
    signal randRotationChanged(real rand)
    signal randScaleChanged(real rand)
    signal maskChanged(real mask)
    signal inputsCountChanged(int count)
    signal seedChanged(int seed)
    signal scaleTileChanged(real scale)
    signal keepProportionChanged(bool keep)
    signal useAlphaChanged(bool use)
    signal depthMaskChanged(bool depth)
    signal bitsChanged(int bitstType)
    signal propertyChangingFinished(string name, var newValue, var oldValue)
    ParamDropDown {
        id: bitsParam
        y: 15
        model: ["8 bits", "16 bits"]
        onCurrentIndexChanged: {
            if(currentIndex == 0) {
                bitsChanged(0)
            }
            else if(currentIndex == 1) {
                bitsChanged(1)
            }
            focus = false
        }
        onActivated: {
            propertyChangingFinished("startBits", currentIndex, oldIndex)
        }
    }
    Item {
        width: parent.width - 40
        height: childrenRect.height
        x: 10
        y: 53
        clip: true
        ParamSlider {
            id: columnsParam
            propertyName: "Columns"
            minimum: 1
            maximum: 20
            step: 1
            onPropertyValueChanged: {
                columnsChanged(columnsParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startColumns", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: rowsParam
            y: 18
            propertyName: "Rows"
            minimum: 1
            maximum: 20
            step: 1
            onPropertyValueChanged: {
                rowsChanged(rowsParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startRows", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: offsetXParam
            y: 51
            propertyName: "Offset X"
            onPropertyValueChanged: {
                offsetXChanged(offsetXParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startOffsetX", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: offsetYParam
            y: 84
            propertyName: "Offset Y"
            onPropertyValueChanged: {
                offsetYChanged(offsetYParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startOffsetY", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: scaleParam
            y: 117
            maximum: 4
            propertyName: "Scale"
            onPropertyValueChanged: {
                scaleTileChanged(propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startTileScale", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: scaleXParam
            y: 150
            maximum: 4
            propertyName: "Scale X"
            onPropertyValueChanged: {
                scaleXChanged(scaleXParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startScaleX", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: scaleYParam
            y: 184
            maximum: 4
            propertyName: "Scale Y"
            onPropertyValueChanged: {
                scaleYChanged(scaleYParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startScaleY", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: rotationParam
            y: 216
            maximum: 360
            step: 1
            propertyName: "Rotation"
            onPropertyValueChanged: {
                rotationAngleChanged(rotationParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startRotation", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: randPositionParam
            y: 249
            propertyName: "Randomizing position"
            onPropertyValueChanged: {
                randPositionChanged(randPositionParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startRandPosition", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: randRotationParam
            y: 282
            propertyName: "Randomizing rotation"
            onPropertyValueChanged: {
                randRotationChanged(randRotationParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startRandRotation", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: randScaleParam
            y: 315
            propertyName: "Randomizing scale"
            onPropertyValueChanged: {
                randScaleChanged(randScaleParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startRandScale", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: maskStrengthParam
            y: 348
            propertyName: "Mask"
            onPropertyValueChanged: {
                maskChanged(maskStrengthParam.propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startMask", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: inputsCountParam
            y: 381
            minimum: 1
            maximum: 6
            step: 1
            propertyName: "Inputs count"
            onPropertyValueChanged: {
                inputsCountChanged(propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startInputsCount", propertyValue, oldValue)
            }
        }
        ParamSlider {
            id: seedParam
            y: 414
            minimum: 1
            maximum: 100
            step: 1
            propertyName: "Seed"
            onPropertyValueChanged: {
                seedChanged(propertyValue)
            }
            onChangingFinished: {
                propertyChangingFinished("startSeed", propertyValue, oldValue)
            }
        }
    }

    ParamCheckbox{
        id: keepProportionParam
        y: 515
        text: qsTr("Keep proportion")
        onCheckedChanged: {
            keepProportionChanged(keepProportionParam.checked)
        }
        onToggled: {
            propertyChangingFinished("startKeepProportion", checked, !checked)
            focus = false
        }
    }
    ParamCheckbox {
        id: depthMaskParam
        y:540
        text: qsTr("Depth mask")
        checked: true
        onCheckedChanged: {
            depthMaskChanged(depthMaskParam.checked)
        }
        onToggled: {
            propertyChangingFinished("startDepthMask", checked, !checked)
            focus = false
        }
    }
    ParamCheckbox {
        id: useAlphaParam
        y: 565
        text: qsTr("Use alpha")
        checked: true
        onCheckedChanged: {
            useAlphaChanged(useAlphaParam.checked)
        }
        onToggled: {
            propertyChangingFinished("startUseAlpha", checked, !checked)
            focus = false
        }
    }
}
