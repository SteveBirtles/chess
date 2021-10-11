#!/bin/bash
g++ -o chess chess.cpp -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -std=c++17