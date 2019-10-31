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

void Themer::styleColorsDark()
{
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 window_color(0.12f, 0.12f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = window_color;

    ImVec4 titlebar_color(0.16f, 0.16f, 0.16f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = titlebar_color;
    style.Colors[ImGuiCol_TitleBg] = titlebar_color;
    style.Colors[ImGuiCol_TitleBgActive] = titlebar_color;

    ImVec4 widget_color(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_Button] = widget_color;
    style.Colors[ImGuiCol_HeaderActive] = widget_color;
    style.Colors[ImGuiCol_FrameBg] = widget_color;
    style.Colors[ImGuiCol_TabUnfocusedActive] = widget_color;

    ImVec4 separator_color(0.24f, 0.24f, 0.24f, 1.0f);
    style.Colors[ImGuiCol_Separator] = separator_color;

    ImVec4 unfocused_color(0.28f, 0.28f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_Tab] = unfocused_color;
    style.Colors[ImGuiCol_TabUnfocused] = unfocused_color;

    ImVec4 hovered_color(0.36f, 0.36f, 0.36f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = hovered_color;
    style.Colors[ImGuiCol_FrameBgHovered] = hovered_color;
    style.Colors[ImGuiCol_HeaderHovered] = hovered_color;
    style.Colors[ImGuiCol_SeparatorHovered] = hovered_color;
    style.Colors[ImGuiCol_TabHovered] = hovered_color;

    ImVec4 active_color(0.2f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = active_color;
    style.Colors[ImGuiCol_FrameBgActive] = active_color;
    style.Colors[ImGuiCol_Header] = active_color;
    style.Colors[ImGuiCol_SeparatorActive] = active_color;
    style.Colors[ImGuiCol_TabActive] = active_color;

    ImVec4 text_color(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = text_color;
}

void Themer::styleColorsLight()
{
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 window_color(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = window_color;

    ImVec4 titlebar_color(0.96f, 0.96f, 0.96f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = titlebar_color;
    style.Colors[ImGuiCol_TitleBg] = titlebar_color;
    style.Colors[ImGuiCol_TitleBgActive] = titlebar_color;

    ImVec4 widget_color(0.92f, 0.92f, 0.92f, 1.0f);
    style.Colors[ImGuiCol_Button] = widget_color;
    style.Colors[ImGuiCol_HeaderActive] = widget_color;
    style.Colors[ImGuiCol_FrameBg] = widget_color;
    style.Colors[ImGuiCol_TabUnfocusedActive] = widget_color;

    ImVec4 separator_color(0.88f, 0.88f, 0.88f, 1.0f);
    style.Colors[ImGuiCol_Separator] = separator_color;

    ImVec4 unfocused_color(0.84f, 0.84f, 0.84f, 1.0f);
    style.Colors[ImGuiCol_Tab] = unfocused_color;
    style.Colors[ImGuiCol_TabUnfocused] = unfocused_color;

    ImVec4 hovered_color(0.72f, 0.72f, 0.72f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = hovered_color;
    style.Colors[ImGuiCol_FrameBgHovered] = hovered_color;
    style.Colors[ImGuiCol_HeaderHovered] = hovered_color;
    style.Colors[ImGuiCol_SeparatorHovered] = hovered_color;
    style.Colors[ImGuiCol_TabHovered] = hovered_color;

    ImVec4 active_color(0.2f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = active_color;
    style.Colors[ImGuiCol_FrameBgActive] = active_color;
    style.Colors[ImGuiCol_Header] = active_color;
    style.Colors[ImGuiCol_SeparatorActive] = active_color;
    style.Colors[ImGuiCol_TabActive] = active_color;

    ImVec4 text_color(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Text] = text_color;
}
