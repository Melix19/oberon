/*
    This file is part of Oberon.

    Copyright (c) 2019-2020 Marco Melorio

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "Theme.h"

#include <imgui.h>

namespace Oberon { namespace Editor {

namespace Theme {

void setStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.ItemSpacing.y = 5.0f;
    style.ItemInnerSpacing.x = 5.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarSize = 10.0f;
}

void setStyleColor(Color color) {
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 titleBarColor;
    ImVec4 windowColor;
    ImVec4 widgetColor;
    ImVec4 hoveredColor;
    ImVec4 activeColor;
    ImVec4 textColor;

    switch(color) {
        case Color::Dark: {
            titleBarColor = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
            windowColor = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
            widgetColor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
            hoveredColor = ImVec4(0.35f, 0.35f, 0.35f, 0.8f);
            activeColor = ImVec4(0.2f, 0.5f, 0.9f, 0.8f);
            textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        } break;
        case Color::Light: {
            titleBarColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            windowColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            widgetColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            hoveredColor = ImVec4(0.75f, 0.75f, 0.75f, 0.8f);
            activeColor = ImVec4(0.2f, 0.5f, 0.9f, 0.8f);
            textColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        } break;
    }

    style.Colors[ImGuiCol_PopupBg] = titleBarColor;
    style.Colors[ImGuiCol_TabUnfocused] = titleBarColor;
    style.Colors[ImGuiCol_Tab] = titleBarColor;
    style.Colors[ImGuiCol_TitleBg] = titleBarColor;
    style.Colors[ImGuiCol_TitleBgActive] = titleBarColor;
    style.Colors[ImGuiCol_TitleBgCollapsed] = titleBarColor;

    style.Colors[ImGuiCol_MenuBarBg] = windowColor;
    style.Colors[ImGuiCol_ScrollbarBg] = windowColor;
    style.Colors[ImGuiCol_TabActive] = windowColor;
    style.Colors[ImGuiCol_TabHovered] = windowColor;
    style.Colors[ImGuiCol_TabUnfocusedActive] = windowColor;
    style.Colors[ImGuiCol_WindowBg] = windowColor;

    style.Colors[ImGuiCol_Button] = widgetColor;
    style.Colors[ImGuiCol_FrameBg] = widgetColor;
    style.Colors[ImGuiCol_Separator] = widgetColor;

    style.Colors[ImGuiCol_ButtonHovered] = hoveredColor;
    style.Colors[ImGuiCol_FrameBgHovered] = hoveredColor;
    style.Colors[ImGuiCol_HeaderHovered] = hoveredColor;
    style.Colors[ImGuiCol_SeparatorHovered] = hoveredColor;

    style.Colors[ImGuiCol_ButtonActive] = activeColor;
    style.Colors[ImGuiCol_FrameBgActive] = activeColor;
    style.Colors[ImGuiCol_Header] = activeColor;
    style.Colors[ImGuiCol_HeaderActive] = activeColor;
    style.Colors[ImGuiCol_SeparatorActive] = activeColor;

    style.Colors[ImGuiCol_Text] = textColor;
}

void setNextItemRightAlign(const char* label, Float spacing) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label);
    ImGui::SameLine(spacing);
    ImGui::SetNextItemWidth(-1);
}

}}}
