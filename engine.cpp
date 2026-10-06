#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cctype>
#include <string>
#include <map>
#include <span>
#include <bit>

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

uint64_t SquareToBit(std::string square){

    int rank = square[1] - '0';
    int file = (square[0] -'a') + 1;
    uint16_t squarebitposition = 8 * (rank - 1) + (8 - file); 
    uint64_t squaremask = (1ULL << squarebitposition);
    
    return squaremask;
    

}
class Board
{

public:
    std::array<uint64_t, total_pieces> board{};
    std::uint64_t WhitePieces = 0;
    std::uint64_t BlackPieces = 0;
    bool ToMove = white;
    std::array<bool, total_castling> castle{};

    std::uint64_t en_passant;

    int halfmoves = 0;

    int fullmoves = 0;

    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    void fenparsing(std::string fen)

    {
        board.fill(0);
        WhitePieces = 0;
        BlackPieces = 0;
        castle.fill(false);
        en_passant = 0;
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
        halfmoves = std::stoi(boardinfo[4]);
        fullmoves = std::stoi(boardinfo[5]);
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

    uint16_t NORMAL_QUIET_MOVE              = 0b0000 << 12;
    uint16_t DOUBLE_PAWN_PUSH               = 0b0001 << 12;
    uint16_t QUEEN_SIDE_CASTLE              = 0b0010 << 12;
    uint16_t KING_SIDE_CASTLE               = 0b0011 << 12;
    uint16_t PROMOTE_TO_KNIGHT_QUIET        = 0b0100 << 12;
    uint16_t PROMOTE_TO_BISHOP_QUIET        = 0b0101 << 12;
    uint16_t PROMOTE_TO_ROOK_QUIET          = 0b0110 << 12;
    uint16_t PROMOTE_TO_QUEEN_QUIET         = 0b0111 << 12;
    uint16_t NORMAL_CAPTURE                 = 0b1000 << 12;
    uint16_t EN_PASSANT                     = 0b1001 << 12;
    uint16_t CAPTURE_AND_PROMOTE_TO_KNIGHT  = 0b1100 << 12;
    uint16_t CAPTURE_AND_PROMOTE_TO_BISHOP  = 0b1101 << 12;
    uint16_t CAPTURE_AND_PROMOTE_TO_ROOK    = 0b1110 << 12;
    uint16_t CAPTURE_AND_PROMOTE_TO_QUEEN   = 0b1111 << 12;
    // 1010 AND 1011 ARE UNUSED currently

    void addmove(uint16_t flag , uint16_t from, uint16_t to){

             movelist.push_back(flag | (from << 6) | to );

    }

     void addpromotion( bool iscapture , uint16_t from, uint16_t to) {
        if(iscapture){
            addmove(CAPTURE_AND_PROMOTE_TO_BISHOP , from , to);
            addmove(CAPTURE_AND_PROMOTE_TO_KNIGHT , from , to);
            addmove(CAPTURE_AND_PROMOTE_TO_QUEEN , from , to);
            addmove(CAPTURE_AND_PROMOTE_TO_ROOK , from , to);
        }

        else{
            addmove(PROMOTE_TO_BISHOP_QUIET , from , to);
            addmove(PROMOTE_TO_KNIGHT_QUIET , from , to);
            addmove(PROMOTE_TO_QUEEN_QUIET , from , to);
            addmove(PROMOTE_TO_ROOK_QUIET , from , to);

        }
    }


    void addpawnmoves(uint16_t from){

        uint64_t sourcemask    = (1ULL<<from);
        bool iswhite       = ((sourcemask & WhitePieces) != 0);
        int  lastrank      = (iswhite)? 8 : 1;
        int  startingrank  = (iswhite)? 2 : 7;
        int  direction     = (iswhite)? +1 : -1;
        uint64_t occupied  = (WhitePieces | BlackPieces);


        // forward moves

        int initialrank            = from / 8 + 1;
        int initialfile            = 8 - (from % 8);
        int forwardrank            = initialrank + direction;
        uint16_t forwardsquare     =  8 * (forwardrank - 1) + (8 - initialfile);
        uint64_t forwardsquaremask = (1ULL<<forwardsquare);



        if((forwardsquaremask & occupied) == 0){

            bool iscapture = false;
            if(forwardrank == lastrank) addpromotion(iscapture , from , forwardsquare);
            else addmove(NORMAL_QUIET_MOVE , from , forwardsquare);

            if(initialrank == startingrank){
                int doublerank        = startingrank + direction*2;
                uint16_t doublesquare = 8 * (doublerank - 1) + (8 - initialfile);
                uint64_t doublemask   = (1ULL << doublesquare);

                if((doublemask & occupied) == 0) addmove(DOUBLE_PAWN_PUSH, from , doublesquare);

            }
        }

        for(int filedirection : {-1 , 1}){

            int newfile = initialfile + filedirection;
            if(newfile < 1 || newfile > 8) continue;

            forwardsquare = 8 * (forwardrank - 1) + (8 - newfile);
            forwardsquaremask = (1ULL<<forwardsquare);

            if((forwardsquaremask & en_passant) != 0){
                addmove(EN_PASSANT , from , forwardsquare);
            }

            else if((forwardsquaremask & occupied) != 0){

                bool isenemywhite = ((forwardsquaremask & WhitePieces) != 0);

                if(iswhite != isenemywhite) {
                    bool iscapture = true;
                    if(forwardrank == lastrank) addpromotion(iscapture , from , forwardsquare);
                    else addmove(NORMAL_CAPTURE , from , forwardsquare);
                }
            }

        }
        
    }

    void nonpawnmovegen(std::span<int> fileoffset, std::span<int> rankoffset, uint16_t sourcesq, bool sourceisN, bool sourceisK, bool checkingcastle)
    {

        int startrank = sourcesq / 8 + 1;
        int startfile = 8 - (sourcesq % 8);
        int totaldirections = fileoffset.size(); // file amd rank offset are of the same size




        bool sourceiswhite = false;
        if ((1ULL << sourcesq) & (WhitePieces))
            sourceiswhite = true;


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

                uint16_t square = 8 * (currentrank - 1) + (8 - currentfile); // number between 0 to 63 , will be used to find the bitboard

                uint16_t flags = 0;

                bool pieceencountered = (((1ULL << square) & (WhitePieces | BlackPieces)) != 0);

                if (pieceencountered)
                {

                    bool targetiswhite = false;
                    if ((1ULL << square) & (WhitePieces))
                        targetiswhite = true;

                    bool targetisrook = ((board[R] & (1ULL << square)) != 0);

                    if (sourceisK && (sourceiswhite == targetiswhite) && targetisrook && checkingcastle)
                    {
                        uint16_t h1 = 0;
                        uint16_t a1 = 7;
                        uint16_t h8 = 56;
                        uint16_t a8 = 63;

                        if(!(square == h1 || square == a1 || square == h8  ||  square == a8)) break;
                        
                
                        if (fileoffset[i] == +1)
                            flags = KING_SIDE_CASTLE;
                        else
                            flags = QUEEN_SIDE_CASTLE;
                    }

                    else if ((sourceiswhite != targetiswhite) && !checkingcastle)
                        flags = NORMAL_CAPTURE;
                    else
                        break;
                }

                else if (checkingcastle)
                    continue;

                uint16_t from = sourcesq << 6;
                uint16_t to = square;

                uint16_t move = (flags | from | to);

                movelist.push_back(move);

                if (pieceencountered || sourceisN || (sourceisK && !checkingcastle))
                    break;
            }
        }
    }



    void GenerateMovesFromSquare(uint16_t sourcesq){

        uint64_t sourcemask = (1ULL<<sourcesq);
        bool sourceisn = false ,  sourceisk = false, checkingcastle = false;

        for(int piece = 0 ; piece < total_pieces ; piece++){

            if((board[piece] & sourcemask) != 0){
                switch(piece){

                    case R:
                         {
                            std::array<int, 4> rankoffset = {+1, -1, 0, 0};
                            std::array<int, 4> fileoffset = {0, 0, +1, -1};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                           }
                           break;

                    case N:
                         {
                            sourceisn = true;
                            std::array<int, 8> rankoffset = {+2, +2, -2, -2, +1, -1, +1, -1};
                            std::array<int, 8> fileoffset = {+1, -1, +1, -1, +2, +2, -2, -2};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                            }
                            break; 
                    case B:
                          {
                            std::array<int, 4> rankoffset = {+1, +1, -1, -1};
                            std::array<int, 4> fileoffset = {+1, -1, -1, +1};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(fileoffsetview, rankoffsetview ,sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                          }
                          break;
                    case Q:
                           {
                            std::array<int, 8> rankoffset = {+1, -1, 0, 0, +1, +1, -1, -1};
                            std::array<int, 8> fileoffset = {0, 0, +1, -1, +1, -1, -1, +1};
                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                           }
                           break;
                    case K:
                           {
                           bool sourceisk = true;

                            std::array<int, 8> rankoffset = {+1, -1, 0, 0, +1, +1, -1, -1};
                            std::array<int, 8> fileoffset = {0, 0, +1, -1, +1, -1, -1, +1};
                    
                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);

                            checkingcastle = true;
                            bool iswhite = ((sourcemask & WhitePieces) !=0);
                            bool kingsidecastle = iswhite? castle[wkside] : castle[bkside];
                            bool queensidecastle = iswhite? castle[wqside] : castle[bqside];

                            std::array<int , 1> castlerankoffset = {0};

                            if(kingsidecastle){
                                std::array<int , 1> castlefileoffset = {+1};
                                std::span<int> castlefile(castlefileoffset);
                                std::span<int> castlerank(castlerankoffset);
                               nonpawnmovegen(castlefile, castlerank, sourcesq, sourceisn , sourceisk , checkingcastle);
                            

                            }

                            if(queensidecastle){
                                std::array<int , 1> castlefileoffset = {-1};
                                std::span<int> castlefile(castlefileoffset);
                                std::span<int> castlerank(castlerankoffset);
                               nonpawnmovegen(castlefile, castlerank, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                            }

                           }
                           break;

                    case P: 
                            addpawnmoves(sourcesq);
                            break;
                }
            }
        }
    }


    void GeneratePseudoLegalMove(){

        movelist.clear();

        for(uint64_t piece : board){

           uint64_t bb = piece;

           while(bb != 0){

            uint16_t from = std::countr_zero(bb);

            bool iswhite = ((1ULL<<from  & WhitePieces) != 0);

            if(iswhite == ToMove)
            GenerateMovesFromSquare(from);

            bb &= bb - 1;

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