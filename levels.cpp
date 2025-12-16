#include "headers/levels.h"


std::vector<std::vector<int> > getLevelLayout(int levelNumber) {
    std::vector<std::vector<int> > layout;

    switch (levelNumber) {
        case 1: {
            // Stage 1
            const int level1Data[6][10] = {
                {1,1,1,0,0,0,0,0,0,0},
                {1,9,1,1,1,1,0,0,0,0},
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
            const int level2Data[7][15] = {
                {1,1,1,3,3,3,3,3,3,3,1,1,0,0,0},
                {1,9,1,3,3,3,3,3,3,3,1,1,0,0,0},
                {1,1,1,1,0,0,0,0,0,1,1,1,0,0,0},
                {1,1,1,0,0,1,1,1,1,3,3,3,3,3,0},
                {1,1,1,0,0,1,1,1,1,3,3,3,3,3,0},
                {0,0,0,0,0,1,2,1,0,0,3,3,1,3,0},
                {0,0,0,0,0,1,1,1,0,0,3,3,3,3,0}
            };
            for (int i = 0; i < 7; ++i) {
                layout.push_back(std::vector<int>(level2Data[i], level2Data[i] + 15));
            }
            break;
        }

        case 3: {
            // Stage 3
            const int level3Data[5][15] = {
                {1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
                {1,1,5,1,0,0,1,1,5,1,0,0,1,2,1},
                {1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
                {1,9,1,1,4,4,1,1,1,1,4,4,1,1,1},
                {1,1,1,1,0,0,1,1,1,1,0,0,0,0,0}
            };
            for (int i = 0; i < 5; ++i) {
                layout.push_back(std::vector<int>(level3Data[i], level3Data[i] + 15));
            }
            break;
        }
        
        default:
            return getLevelLayout(1);
    }

    return layout;
}