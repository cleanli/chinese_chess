#ifndef _COORDINATE_TRANS_H
#define _COORDINATE_TRANS_H
#include "ch_chess.h"

#define CELL_SIZE (364.0f/8)
#define CHESS_BOARD_X_START 104.0f
#define CHESS_BOARD_Y_START 154.0f
#define CHESS_BOARD_X_END 468.0f
#define CHESS_BOARD_Y_END 556.0f

class c_coordinate_trans
{
    public:
        c_coordinate_trans(bool b)
          : revert(b)
        {
        }
        void set_revert(bool b){revert = b;}
        bool is_in_chessboard(int x, int y)
        {
            return (x > (CHESS_BOARD_X_START - CELL_SIZE/2)) &&
                (x < (CHESS_BOARD_X_END + CELL_SIZE/2)) &&
                (y > (CHESS_BOARD_Y_START - CELL_SIZE/2)) &&
                (y < (CHESS_BOARD_Y_END + CELL_SIZE/2));
        }
        float screen_to_chess_x(float x)
        {
            if(revert){
                return MAX_CHS_BOARD_X - display_to_chess_x(x);
            }
            else{
                return display_to_chess_x(x);
            }
        }
        float screen_to_chess_y(float y)
        {
            if(revert){
                return MAX_CHS_BOARD_Y - display_to_chess_y(y);
            }
            else{
                return display_to_chess_y(y);
            }
        }
        float chess_to_screen_x(float x)
        {
            if(revert){
                return chess_to_display_x(MAX_CHS_BOARD_X - x -1);
            }
            else{
                return chess_to_display_x(x);
            }
        }
        float chess_to_screen_y(float y)
        {
            if(revert){
                return chess_to_display_y(MAX_CHS_BOARD_Y - y -1);
            }
            else{
                return chess_to_display_y(y);
            }
        }
    private:
        bool revert;
        float chess_to_display_x(float cx)
        {
            return CHESS_BOARD_X_START + CELL_SIZE * cx;
        }

        float chess_to_display_y(float cy)
        {
            return CHESS_BOARD_Y_START + CELL_SIZE * cy;
        }

        float display_to_chess_x(float x)
        {
            return (x + CELL_SIZE/2 - CHESS_BOARD_X_START)/CELL_SIZE;
        }

        float display_to_chess_y(float y)
        {
            return (y + CELL_SIZE/2 - CHESS_BOARD_Y_START)/CELL_SIZE;
        }

};

#endif
