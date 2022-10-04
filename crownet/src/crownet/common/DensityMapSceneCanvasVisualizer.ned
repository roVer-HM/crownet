//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package crownet.common;

import inet.visualizer.base.SceneVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

// This module visualizes the density map with the selecetd Area of Intrest.


simple DensityMapSceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(crownet::DensityMapSceneCanvasVisualizer);
        string coordConverterModule = default("coordConverterModule"); // the geographic coordinate system, refers to a top level module by default

        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        bool adjustBackgroundBox = default(true); // if true, sets background box (bgb tag) to match map bounds
}