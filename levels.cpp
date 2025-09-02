#include "levels.h"


std::vector<std::vector<int> > getLevelLayout(int levelNumber) {
    std::vector<std::vector<int> > layout;

    switch (levelNumber) {
        case 1: {
            // Stage 1
            const int level1Data[6][10] = {
                {1,1,1,0,0,0,0,0,0,0},
                {1,1,1,1,1,1,0,0,0,0},
                {1,1,1,1,1,1,1,1,1,0},
                {0,1,1,1,1,1,1,1,1,1},
                {0,0,0,0,0,1,1,2,1,1},
                {0,0,0,0,0,0,1,1,1,0}
            };
            for (int i = 0; i < 6; ++i) {
                layout.push_back(std::vector<int>(level1Data[i], level1Data[i] + 10));
            }
            break;
        }

        case 2: {
            // Stage 2
            const int level2Data[5][5] = {
                {1,1,1,1,1},
                {1,0,0,0,1},
                {1,0,1,1,1},
                {1,1,1,0,2},
                {0,0,1,1,1}
            };
            for (int i = 0; i < 5; ++i) {
                layout.push_back(std::vector<int>(level2Data[i], level2Data[i] + 5));
            }
            break;
        }
        
        default:
            return getLevelLayout(1);
    }

    return layout;
}