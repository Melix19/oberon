/*
    MIT License

    Copyright (c) 2019 Marco Melorio

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

#include "Themer.hpp"

void Themer::styleColorsDark() {
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 windowColor(0.12f, 0.12f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = windowColor;

    ImVec4 titlebarColor(0.16f, 0.16f, 0.16f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = titlebarColor;
    style.Colors[ImGuiCol_TitleBg] = titlebarColor;
    style.Colors[ImGuiCol_TitleBgActive] = titlebarColor;

    ImVec4 widgetColor(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Button] = widgetColor;
    style.Colors[ImGuiCol_HeaderActive] = widgetColor;
    style.Colors[ImGuiCol_FrameBg] = widgetColor;
    style.Colors[ImGuiCol_TabUnfocusedActive] = widgetColor;

    ImVec4 separatorColor(0.24f, 0.24f, 0.24f, 1.0f);
    style.Colors[ImGuiCol_Separator] = separatorColor;

    ImVec4 unfocusedColor(0.28f, 0.28f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_Tab] = unfocusedColor;
    style.Colors[ImGuiCol_TabUnfocused] = unfocusedColor;

    ImVec4 hoveredColor(0.36f, 0.36f, 0.36f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = hoveredColor;
    style.Colors[ImGuiCol_FrameBgHovered] = hoveredColor;
    style.Colors[ImGuiCol_HeaderHovered] = hoveredColor;
    style.Colors[ImGuiCol_SeparatorHovered] = hoveredColor;
    style.Colors[ImGuiCol_TabHovered] = hoveredColor;

    ImVec4 activeColor(0.2f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = activeColor;
    style.Colors[ImGuiCol_FrameBgActive] = activeColor;
    style.Colors[ImGuiCol_Header] = activeColor;
    style.Colors[ImGuiCol_SeparatorActive] = activeColor;
    style.Colors[ImGuiCol_TabActive] = activeColor;

    ImVec4 textColor(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = textColor;
}

void Themer::styleColorsLight() {
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 windowColor(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = windowColor;

    ImVec4 titlebarColor(0.96f, 0.96f, 0.96f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = titlebarColor;
    style.Colors[ImGuiCol_TitleBg] = titlebarColor;
    style.Colors[ImGuiCol_TitleBgActive] = titlebarColor;

    ImVec4 widgetColor(0.92f, 0.92f, 0.92f, 1.0f);
    style.Colors[ImGuiCol_Button] = widgetColor;
    style.Colors[ImGuiCol_HeaderActive] = widgetColor;
    style.Colors[ImGuiCol_FrameBg] = widgetColor;
    style.Colors[ImGuiCol_TabUnfocusedActive] = widgetColor;

    ImVec4 separatorColor(0.88f, 0.88f, 0.88f, 1.0f);
    style.Colors[ImGuiCol_Separator] = separatorColor;

    ImVec4 unfocusedColor(0.84f, 0.84f, 0.84f, 1.0f);
    style.Colors[ImGuiCol_Tab] = unfocusedColor;
    style.Colors[ImGuiCol_TabUnfocused] = unfocusedColor;

    ImVec4 hoveredColor(0.72f, 0.72f, 0.72f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = hoveredColor;
    style.Colors[ImGuiCol_FrameBgHovered] = hoveredColor;
    style.Colors[ImGuiCol_HeaderHovered] = hoveredColor;
    style.Colors[ImGuiCol_SeparatorHovered] = hoveredColor;
    style.Colors[ImGuiCol_TabHovered] = hoveredColor;

    ImVec4 activeColor(0.2f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = activeColor;
    style.Colors[ImGuiCol_FrameBgActive] = activeColor;
    style.Colors[ImGuiCol_Header] = activeColor;
    style.Colors[ImGuiCol_SeparatorActive] = activeColor;
    style.Colors[ImGuiCol_TabActive] = activeColor;

    ImVec4 textColor(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = textColor;
}
