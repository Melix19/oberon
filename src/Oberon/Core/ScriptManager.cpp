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

#include "ScriptManager.h"

void ScriptManager::loadScripts(ScriptGroup& scripts) {
    for(std::size_t i = 0; i != scripts.size(); ++i) {
        Script& script = scripts[i];

        if(_manager.loadState(script.name()) & PluginManager::LoadState::NotLoaded)
            _manager.load(script.name());

        Object3D* object = reinterpret_cast<Object3D*>(&script.object());
        Containers::arrayAppend(_scripts, std::make_pair(_manager.instantiate(script.name()), object));
    }
}

void ScriptManager::unloadScripts() {
    for(auto& script: _scripts)
        script.first.reset(nullptr);
    Containers::arrayResize(_scripts, 0);

    for(auto& plugin: _manager.pluginList())
        _manager.unload(plugin);
}

void ScriptManager::update(Float deltaTime) {
    for(auto& script: _scripts)
        script.first->update(script.second, deltaTime);
}
