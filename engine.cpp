#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cctype>
#include <string>
#include <map>

enum pieces
{
    R,
    N,
    B,
    Q,
    K,
    P,
    total_pieces
};
enum side
{
    white,
    black
};

enum castlingrights
{
    wkside,
    wqside,
    bkside,
    bqside,
    total_castling
};

int SquareToBit(std::string square)
{
    return ((8 * ((square[1] - '1')) + ('h' - square[0])));
}

class Board
{

public:
    std::array<uint64_t, total_pieces> board{};

    std::uint64_t WhitePieces = 0;

    std::uint64_t BlackPieces = 0;

    bool ToMove = white;

    std::array<bool, total_castling> castle{};

    int en_passant = -1;

    int halfmoves = 0;

    int fullmoves = 0;

    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq e3 0 1";

    void fenparsing(std::string fen)

    {
        board.fill(0);
        WhitePieces = 0;
        BlackPieces = 0;
        castle.fill(false);
        en_passant = -1;
        std::vector<std::string> boardinfo;

        std::string temp;

        for (auto &c : fen)
        {

            if (c == ' ')
            {
                boardinfo.push_back(temp);
                temp = "";
            }
            else
                temp += c;
        }
        boardinfo.push_back(temp);

        int square = 63;

        for (auto &c : boardinfo[0])
        {
            if (c == '/')
                continue;

            if (isdigit(c))
                square -= (c - '0');

            else
            {

                std::uint64_t squaretoactivate = static_cast<std::uint64_t>((1ULL << square));

                if (isupper(c))
                    WhitePieces |= squaretoactivate;

                else
                    BlackPieces |= squaretoactivate;

                switch ((char)toupper(c))
                {
                case 'R':
                    board[R] |= squaretoactivate;
                    break;
                case 'N':
                    board[N] |= squaretoactivate;
                    break;
                case 'B':
                    board[B] |= squaretoactivate;
                    break;
                case 'Q':
                    board[Q] |= squaretoactivate;
                    break;
                case 'K':
                    board[K] |= squaretoactivate;
                    break;
                case 'P':
                    board[P] |= squaretoactivate;
                    break;
                }

                square--;
            }
        }

        ToMove = (boardinfo[1] == "b") ? black : white;

        for (auto &c : boardinfo[2])
        {
            if (c == '-')
                break;

            switch (c)
            {
            case 'K':
                castle[wkside] = true;
                break;
            case 'Q':
                castle[wqside] = true;
                break;
            case 'k':
                castle[bkside] = true;
                break;
            case 'q':
                castle[bqside] = true;
                break;
            }
        }

        if (boardinfo[3] != "-")
            en_passant = SquareToBit(boardinfo[3]);
        halfmoves = stoi(boardinfo[4]);
        fullmoves = stoi(boardinfo[5]);
    }

    void printboard()
    {

        std::map<pieces, char> piecetochar = {
            {R, 'R'},
            {N, 'N'},
            {B, 'B'},
            {Q, 'Q'},
            {K, 'K'},
            {P, 'P'}};

        int square = 63;
        int ranks = 8;
        int files = 8;

        for (int i = ranks; i >= 1; i--)
        {

            for (int j = files; j >= 1; j--)
            {

                if (((1ULL << square) & (WhitePieces | BlackPieces)) != 0)
                {

                    for (int piece{R}; piece < total_pieces; piece++)
                    {

                        if (((1ULL << square) & board[piece]) != 0)
                        {

                            if (((1ULL << square) & (WhitePieces)) != 0)
                                std::cout << piecetochar[static_cast<pieces>(piece)] << " ";

                            else
                                std::cout << static_cast<char>(std::tolower(piecetochar[static_cast<pieces>(piece)])) << " ";
                        }
                    }
                }

                else
                    std::cout << "e" << " ";

                square--;
            }

            std::cout << "\n";
        }
    }

    std::vector<uint16_t> movelist;

    void bishopmoves(uint16_t bishopsourcesquare) 
    {

        int startrank = bishopsourcesquare / 8 + 1;
        int startfile = 8 - (bishopsourcesquare % 8);
        std::array<int, 4> rankoffset = {+1, +1, -1, -1};
        std::array<int, 4> fileoffset = {+1, -1, -1, +1};
        uint16_t  CAPTURE_FLAG = 0b1000 << 12;

        int totaldirections = 4;

        for (int i = 0; i < totaldirections; i++)
        {

            int currentrank = startrank;
            int currentfile = startfile;



            while (true)
            {
                currentrank += (rankoffset[i]);
                currentfile += (fileoffset[i]);

                if (currentrank > 8 || currentrank < 1 || currentfile > 8 || currentfile < 1)
                    break;

                uint16_t square = 8 * (currentrank - 1) + (8 - currentfile); 

                uint16_t flags = 0;

                bool pieceencountered = (((1ULL << square) & (WhitePieces | BlackPieces)) != 0);

                if (pieceencountered)
                {

                    bool sourceiswhite = false;
                    bool targetiswhite = false;

                    if ((1ULL << bishopsourcesquare) & (WhitePieces))
                        sourceiswhite = true;
                    if ((1ULL << square) & (WhitePieces))
                        targetiswhite = true;

                    if (sourceiswhite != targetiswhite)
                        flags = CAPTURE_FLAG;
                    else
                        break;
                }

                uint16_t from = bishopsourcesquare << 6;
                uint16_t to = square;

                uint16_t move = (flags | from | to);

                movelist.push_back(move);

                if (pieceencountered)
                    break;
            }
        }
    }
};

int main()
{

    Board board1;
    board1.fenparsing(board1.fen);
    board1.printboard();
}