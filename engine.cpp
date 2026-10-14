#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <cctype>
#include <string>
#include <map>
#include <span>
#include <bit>
#include <algorithm>

enum Pieces
{
    EMPTY = -1,
    R,
    N,
    B,
    Q,
    K,
    P,
    total_pieces
    
};
enum Color
{
    colorless = -1,
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

std::string BitToSquare(uint16_t square) {
    int rank = square / 8 + 1;
    int file = 8 - (square % 8);

    char rankChar = '0' + rank;
    char fileChar = 'a' + file - 1;

    std::string result;
    result += fileChar;
    result += rankChar;
    return result;
}


const int WHITE_OCCUPIED  = total_pieces;
const int BLACK_OCCUPIED  = total_pieces + 1;
const int TOTAL_BITBOARDS = total_pieces + 2;


class Board
{
public:

    
    struct Position{
     std::array<uint64_t, total_pieces + 2> Board{};
     std::array<bool, total_castling> Castle{};
     bool ToMove = white;
     std::uint64_t En_passant;
     int Halfmoves = 0;
     int Fullmoves = 0;
     std::string Startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    };


    Position position{};
    

    void fenparsing(std::string fen)

    {
        position.Board.fill(0);
        position.Board[WHITE_OCCUPIED] = 0;
        position.Board[BLACK_OCCUPIED] = 0;

        position.Castle.fill(false);
        position.En_passant = 0;
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
                    position.Board[WHITE_OCCUPIED] |= squaretoactivate;

                else
                    position.Board[BLACK_OCCUPIED] |= squaretoactivate;

                switch ((char)toupper(c))
                {
                case 'R':
                    position.Board[R] |= squaretoactivate;
                    break;
                case 'N':
                    position.Board[N] |= squaretoactivate;
                    break;
                case 'B':
                    position.Board[B] |= squaretoactivate;
                    break;
                case 'Q':
                    position.Board[Q] |= squaretoactivate;
                    break;
                case 'K':
                    position.Board[K] |= squaretoactivate;
                    break;
                case 'P':
                    position.Board[P] |= squaretoactivate;
                    break;
                }

                square--;
            }
        }

        position.ToMove = (boardinfo[1] == "b") ? black : white;

        for (auto &c : boardinfo[2])
        {
            if (c == '-')
                break;

            switch (c)
            {
            case 'K':
                position.Castle[wkside] = true;
                break;
            case 'Q':
                position.Castle[wqside] = true;
                break;
            case 'k':
                position.Castle[bkside] = true;
                break;
            case 'q':
                position.Castle[bqside] = true;
                break;
            }
        }

        if (boardinfo[3] != "-")
           position.En_passant = SquareToBit(boardinfo[3]);

        position.Halfmoves = std::stoi(boardinfo[4]);
        position.Fullmoves = std::stoi(boardinfo[5]);
    }


    void PrintBoard(std::array<uint64_t, total_pieces + 2> Board){

        std::array<char , 64> BoardRepresentation =
        {
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e',
            'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e' , 'e'
            
        };

        std::array<char, 6> PieceToChar = {'R' , 'N' , 'B' , 'Q' , 'K' , 'P'};


        for(int Piece = R ; Piece < total_pieces ; Piece++ ){

            uint64_t bb = Board[Piece];

            while(bb != 0){

              uint16_t from = std::countr_zero(bb);
              bool IsWhite = ((1ULL<<from  & Board[WHITE_OCCUPIED]) != 0);
              uint16_t LastSquareBit = 63;
              BoardRepresentation[LastSquareBit - from] = (IsWhite)?(PieceToChar[Piece]):((char)tolower(PieceToChar[Piece]));
              bb &= bb - 1;
              
            }

        }

        PrintArray(BoardRepresentation);


    }

    void PrintArray(std::array<char , 64> BoardRepresentation){
                int square = 0;

        for(int i = 0 ; i < 8 ; i++){
            for(int j = 0 ; j < 8 ; j++){
                std::cout<<BoardRepresentation[square]<<" ";
                square++;
            }
            std::cout<<" "<<std::endl;
        }

        std::cout<<" "<<std::endl;

    }

    std::vector<uint16_t> movelist;

    static constexpr uint16_t NORMAL_QUIET_MOVE              = 0b0000 << 12;
    static constexpr uint16_t DOUBLE_PAWN_PUSH               = 0b0001 << 12;
    static constexpr uint16_t QUEEN_SIDE_CASTLE              = 0b0010 << 12;
    static constexpr uint16_t KING_SIDE_CASTLE               = 0b0011 << 12;
    static constexpr uint16_t PROMOTE_TO_KNIGHT_QUIET        = 0b0100 << 12;
    static constexpr uint16_t PROMOTE_TO_BISHOP_QUIET        = 0b0101 << 12;
    static constexpr uint16_t PROMOTE_TO_ROOK_QUIET          = 0b0110 << 12;
    static constexpr uint16_t PROMOTE_TO_QUEEN_QUIET         = 0b0111 << 12;
    static constexpr uint16_t NORMAL_CAPTURE                 = 0b1000 << 12;
    static constexpr uint16_t EN_PASSANT                     = 0b1001 << 12;
    static constexpr uint16_t CAPTURE_AND_PROMOTE_TO_KNIGHT  = 0b1100 << 12;
    static constexpr uint16_t CAPTURE_AND_PROMOTE_TO_BISHOP  = 0b1101 << 12;
    static constexpr uint16_t CAPTURE_AND_PROMOTE_TO_ROOK    = 0b1110 << 12;
    static constexpr uint16_t CAPTURE_AND_PROMOTE_TO_QUEEN   = 0b1111 << 12;

    // 1010 AND 1011 ARE UNUSED currently

    void addmove(Position &p , uint16_t flag , uint16_t from, uint16_t to){

             uint16_t Move = (flag | (from << 6) | to );

             if(!LeavesKingInCheck(p , Move))
                 movelist.push_back(Move);

    }

     void addpromotion( Position &p , bool iscapture , uint16_t from, uint16_t to) {
        if(iscapture){
            addmove(p ,CAPTURE_AND_PROMOTE_TO_BISHOP , from , to);
            addmove(p ,CAPTURE_AND_PROMOTE_TO_KNIGHT , from , to);
            addmove(p , CAPTURE_AND_PROMOTE_TO_QUEEN  , from , to);
            addmove(p ,CAPTURE_AND_PROMOTE_TO_ROOK   , from , to);
        }

        else{
            addmove(p , PROMOTE_TO_BISHOP_QUIET , from , to);
            addmove(p ,PROMOTE_TO_KNIGHT_QUIET , from , to);
            addmove(p ,PROMOTE_TO_QUEEN_QUIET  , from , to);
            addmove(p ,PROMOTE_TO_ROOK_QUIET   , from , to);
 
        }
    }


    void addpawnmoves(Position &p , uint16_t from){

        uint64_t sourcemask    = (1ULL<<from);
        bool iswhite       = ((sourcemask & p.Board[WHITE_OCCUPIED]) != 0);
        int  lastrank      = (iswhite)? 8 : 1;
        int  startingrank  = (iswhite)? 2 : 7;
        int  direction     = (iswhite)? +1 : -1;
        uint64_t occupied  = (p.Board[WHITE_OCCUPIED] | p.Board[BLACK_OCCUPIED]);


        // forward moves

        int initialrank            = from / 8 + 1;
        int initialfile            = 8 - (from % 8);
        int forwardrank            = initialrank + direction;
        uint16_t forwardsquare     =  8 * (forwardrank - 1) + (8 - initialfile);
        uint64_t forwardsquaremask = (1ULL<<forwardsquare);



        if((forwardsquaremask & occupied) == 0){

            bool iscapture = false;
            if(forwardrank == lastrank) addpromotion(p ,iscapture , from , forwardsquare);
            else addmove(p ,NORMAL_QUIET_MOVE , from , forwardsquare);

            if(initialrank == startingrank){
                int doublerank        = startingrank + direction*2;
                uint16_t doublesquare = 8 * (doublerank - 1) + (8 - initialfile);
                uint64_t doublemask   = (1ULL << doublesquare);

                if((doublemask & occupied) == 0) addmove(p ,DOUBLE_PAWN_PUSH, from , doublesquare);

            }
        }

        for(int filedirection : {-1 , 1}){

            int newfile = initialfile + filedirection;
            if(newfile < 1 || newfile > 8) continue;

            forwardsquare = 8 * (forwardrank - 1) + (8 - newfile);
            forwardsquaremask = (1ULL<<forwardsquare);

            if((forwardsquaremask & p.En_passant) != 0){
                addmove(p ,EN_PASSANT , from , forwardsquare);
            }

            else if((forwardsquaremask & occupied) != 0){

                bool isenemywhite = ((forwardsquaremask & p.Board[WHITE_OCCUPIED]) != 0);

                if(iswhite != isenemywhite) {
                    bool iscapture = true;
                    if(forwardrank == lastrank) addpromotion(p ,iscapture , from , forwardsquare);
                    else addmove(p ,NORMAL_CAPTURE , from , forwardsquare);
                }
            }

        }
        
    }

    void nonpawnmovegen(Position &p ,std::span<int> fileoffset, std::span<int> rankoffset, uint16_t sourcesq, bool sourceisN, bool sourceisK, bool checkingcastle)
    {

        int startrank = sourcesq / 8 + 1;
        int startfile = 8 - (sourcesq % 8);
        int totaldirections = fileoffset.size(); // file amd rank offset are of the same size


        bool sourceiswhite = false;
        if ((1ULL << sourcesq) & (p.Board[WHITE_OCCUPIED]))
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

                bool pieceencountered = (((1ULL << square) & (p.Board[WHITE_OCCUPIED] | p.Board[BLACK_OCCUPIED])) != 0);

                if (pieceencountered)
                {

                    bool targetiswhite = false;
                    if ((1ULL << square) & (p.Board[WHITE_OCCUPIED]))
                        targetiswhite = true;

                    bool targetisrook = ((p.Board[R] & (1ULL << square)) != 0);

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

                else if (checkingcastle){
                    uint16_t from = sourcesq << 6;
                    uint16_t to = square;

                    uint16_t move = (flags | from | to);

                    if(LeavesKingInCheck(p , move)) break;

                    else continue;
                }

                uint16_t from = sourcesq << 6;
                uint16_t to = square;

                uint16_t move = (flags | from | to);

                if(!LeavesKingInCheck(p , move))
                movelist.push_back(move);

                if (pieceencountered || sourceisN || (sourceisK && !checkingcastle))
                    break;
            }
        }
    }



    void GenerateMovesFromSquare(Position &p , uint16_t sourcesq){

        uint64_t sourcemask = (1ULL<<sourcesq);
        bool sourceisn = false ,  sourceisk = false, checkingcastle = false;

        for(int piece = 0 ; piece < total_pieces ; piece++){

            if((p.Board[piece] & sourcemask) != 0){
                switch(piece){

                    case R:
                         {
                            std::array<int, 4> rankoffset = {+1, -1, 0, 0};
                            std::array<int, 4> fileoffset = {0, 0, +1, -1};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(p ,fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                           }
                           break;

                    case N:
                         {
                            sourceisn = true;
                            std::array<int, 8> rankoffset = {+2, +2, -2, -2, +1, -1, +1, -1};
                            std::array<int, 8> fileoffset = {+1, -1, +1, -1, +2, +2, -2, -2};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(p ,fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                            }
                            break; 
                    case B:
                          {
                            std::array<int, 4> rankoffset = {+1, +1, -1, -1};
                            std::array<int, 4> fileoffset = {+1, -1, -1, +1};

                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(p ,fileoffsetview, rankoffsetview ,sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                          }
                          break;
                    case Q:
                           {
                            std::array<int, 8> rankoffset = {+1, -1, 0, 0, +1, +1, -1, -1};
                            std::array<int, 8> fileoffset = {0, 0, +1, -1, +1, -1, -1, +1};
                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(p ,fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                           }
                           break;
                    case K:
                           {
                           bool sourceisk = true;

                            std::array<int, 8> rankoffset = {+1, -1, 0, 0, +1, +1, -1, -1};
                            std::array<int, 8> fileoffset = {0, 0, +1, -1, +1, -1, -1, +1};
                    
                            std::span<int> fileoffsetview(fileoffset);
                            std::span<int> rankoffsetview(rankoffset);

                            nonpawnmovegen(p ,fileoffsetview, rankoffsetview, sourcesq, sourceisn , sourceisk , checkingcastle);

                            checkingcastle = true;
                            bool iswhite = ((sourcemask & p.Board[WHITE_OCCUPIED]) !=0);
                            bool kingsidecastle = iswhite? p.Castle[wkside] : p.Castle[bkside];
                            bool queensidecastle = iswhite? p.Castle[wqside] : p.Castle[bqside];

                            std::array<int , 1> castlerankoffset = {0};

                            if(kingsidecastle){
                                std::array<int , 1> castlefileoffset = {+1};
                                std::span<int> castlefile(castlefileoffset);
                                std::span<int> castlerank(castlerankoffset);
                               nonpawnmovegen(p ,castlefile, castlerank, sourcesq, sourceisn , sourceisk , checkingcastle);
                            

                            }

                            if(queensidecastle){
                                std::array<int , 1> castlefileoffset = {-1};
                                std::span<int> castlefile(castlefileoffset);
                                std::span<int> castlerank(castlerankoffset);
                               nonpawnmovegen(p ,castlefile, castlerank, sourcesq, sourceisn , sourceisk , checkingcastle);
                            
                            }

                           }
                           break;

                    case P: 
                            addpawnmoves(p , sourcesq);
                            break;
                }
            }
        }
    }

    void PrintHumanReadableMoveList(){

        for(uint16_t &move : movelist){
            std::string from  =  BitToSquare(FromSquareDecoder(move));
            std::string to    =  BitToSquare(ToSquareDecoder(move)); 
            std::cout<<from + to<<std::endl;
        }

    }

    void GenerateLegalMoves(Position &p){

        movelist.clear();

        for(int Piece = R ; Piece < total_pieces ; Piece++){

           
           uint64_t bb = p.Board[Piece];

           while(bb != 0){

            uint16_t from = std::countr_zero(bb);

            bool iswhite = ((1ULL<<from  & p.Board[WHITE_OCCUPIED]) != 0)? white : black;

            if(iswhite == p.ToMove)
            GenerateMovesFromSquare(p , from);

            bb &= bb - 1;

           }
        }
    }


    bool IsSquareAttacked(std::span<uint64_t>board , uint16_t SourceSq , bool knightscan){


        uint64_t SourceMask = (1ULL<<SourceSq);
    
        int startrank        =  SourceSq / 8 + 1;
        int startfile        =  8 - (SourceSq % 8);
        bool IsWhite         = ((SourceMask & board[WHITE_OCCUPIED]) != 0);
        bool IsEmpty         = (SourceMask & (board[WHITE_OCCUPIED] | board[BLACK_OCCUPIED]));
    
        
        std::vector<int> fileoffset = (knightscan) ? 
                                      std::vector<int>{+1, -1, +1, -1, +2, +2, -2, -2}:
                                      std::vector<int>{0, 0, +1, -1, +1, -1, -1, +1  };

        std::vector<int> rankoffset = (knightscan) ? 
                                      std::vector<int>{+2, +2, -2, -2, +1, -1, +1, -1}:
                                      std::vector<int>{+1, -1, 0, 0, +1, +1, -1, -1  };

        int totaldirections = fileoffset.size();
         
            for(int i = 0 ; i < totaldirections ; i++){

                int currentrank = startrank;
                int currentfile = startfile;

                // this while loop explores a particular direction until it encounters a piece or hits boundary 
                // if it hits an opponent piece it returns true otherwise it searches other direction
                while(true){


                    currentrank += rankoffset[i];
                    currentfile += fileoffset[i];
                    if (currentrank > 8 || currentrank < 1 || currentfile > 8 || currentfile < 1) break;

                    uint16_t square       = 8 * (currentrank - 1) + (8 - currentfile);

                    bool pieceencountered = (((1ULL << square) & (board[WHITE_OCCUPIED] | board[BLACK_OCCUPIED])) != 0);
                    bool EnemyIsWhite    = (((1ULL<<square) & board[WHITE_OCCUPIED]) != 0);

                    if(pieceencountered && ((EnemyIsWhite != IsWhite))){
                        
                        bool isopponentrook   = (((1ULL<<square) & (board[R]))!= 0);
                        bool isopponentpawn   = (((1ULL<<square) & (board[P]))!= 0);
                        bool isopponentknight = (((1ULL<<square) & (board[N]))!= 0);
                        bool isopponentqueen  = (((1ULL<<square) & (board[Q]))!= 0);
                        bool isopponentbishop = (((1ULL<<square) & (board[B]))!= 0);
                        bool isopponentking   = (((1ULL<<square) & (board[K]))!= 0);

                        // these two are only relevant for pawn attacks
                        bool onerankahead     = (IsWhite)? (currentrank - startrank == +1) : (currentrank - startrank == -1);
                        bool onefileahead     = (currentfile - startfile == 1 || currentfile - startfile == -1);
    
                        // for knight
                        bool isknightsquare   = ( ((abs(currentrank - startrank) == 2) && abs(currentfile - startfile) == 1) || 
                                                ((abs(currentrank - startrank) == 1) && abs(currentfile - startfile) == 2) );

                        // for bishop and rooks 
                        bool isdiagonal       = (((abs(currentfile - startfile) == abs(currentrank - startrank )) && !isknightsquare));

                        //for king
                        bool onesquareaway    = ((abs(currentrank - startrank) <= 1) && (abs(currentfile - startfile) <= 1));
                                                

                        bool validpawnmove    = (isopponentpawn   && onerankahead && onefileahead);
                        bool validrookmove    = (isopponentrook   && !isdiagonal);
                        bool validknightmove  = (isopponentknight && isknightsquare);
                        bool validbishopmove  = (isopponentbishop && isdiagonal && !isknightsquare);
                        bool validqueenmove   = (isopponentqueen  && !isknightsquare);
                        bool validkingmove    = (isopponentking   && !isknightsquare && onesquareaway);

                        if(validpawnmove || validrookmove || validknightmove || validbishopmove || validqueenmove || validkingmove) return true;
                        else break;


                    }
                    
                    else if((pieceencountered && (EnemyIsWhite == IsWhite)) || knightscan ) break;
            
                }
            }

            return false;
    }

    bool IsSquareAttacked(std::span<uint64_t>board , uint16_t SourceSq){

        bool CheckForKnight = true;
        bool NormalScan     = IsSquareAttacked(board , SourceSq , CheckForKnight);
             CheckForKnight  = false;
        bool KnightScan     = IsSquareAttacked(board , SourceSq , CheckForKnight );
        return (NormalScan || KnightScan);

    }

    bool IsWhiteInCheck(std::span<uint64_t>board){
        uint64_t WhiteKingMask = (board[K] & board[WHITE_OCCUPIED]);
        uint16_t WhiteKingSquare = std::countr_zero(WhiteKingMask);

        bool WhiteChecked = IsSquareAttacked(board , WhiteKingSquare);

        return (WhiteChecked);

    }

    bool IsBlackInCheck(std::span<uint64_t>board){

        uint64_t BlackKingMask = (board[K] & board[BLACK_OCCUPIED]);
        uint16_t BlackKingSquare = std::countr_zero(BlackKingMask);
        bool BlackChecked = IsSquareAttacked(board , BlackKingSquare);

        return (BlackChecked);

    }



    uint16_t FlagDecoder(uint16_t move){
        return ((0b1111 << 12) & move);
    }

    uint16_t FromSquareDecoder(uint16_t move){
        return (((0b111111 << 6) & move) >> 6);
    }

    uint16_t ToSquareDecoder(uint16_t move){
        return (0b111111 & move);
    }


    Pieces PieceOccupyingSquare(std::span<uint64_t>BoardView , uint16_t Square){
        
        uint64_t SquareMask = (1ULL << Square);

        for(int piece = 0 ; piece<total_pieces ; piece++){
            if((SquareMask & BoardView[piece]) != 0) return (static_cast<Pieces>(piece));
        }

        return EMPTY;
    }


    void Relocate(Position &p , int FromColor ,  uint16_t From , uint16_t To , Pieces OldPiece , Pieces NewPiece){


             p.Board[OldPiece]  &= ~(1ULL << From);
             p.Board[FromColor] &= ~(1ULL << From);
             p.Board[NewPiece]  |=  (1ULL << To);
             p.Board[FromColor] |=  (1ULL << To);
    

    }

    void RemovePiece(Position &p , Pieces PieceToRemove , uint16_t RemovalSquare , int Color){
             p.Board[PieceToRemove]  &= ~(1ULL << RemovalSquare);
             p.Board[Color]          &= ~(1ULL << RemovalSquare);
    }




    Position MakeMove (Position p , uint16_t Move){

        uint16_t Flag     = FlagDecoder(Move);
        uint16_t From     = FromSquareDecoder(Move);
        uint16_t To       = ToSquareDecoder(Move);

        std::span<uint64_t>BoardView(p.Board);
    
        const int FromColor  = (((1ULL<<From) & p.Board[WHITE_OCCUPIED]) != 0)? WHITE_OCCUPIED : BLACK_OCCUPIED;
        const int ToColor    = (((1ULL<<To)   & p.Board[WHITE_OCCUPIED]) != 0)? WHITE_OCCUPIED : BLACK_OCCUPIED;
        
        Pieces FromPiece  = PieceOccupyingSquare(BoardView , From);
        Pieces ToPiece    = PieceOccupyingSquare(BoardView , To);



        switch(Flag){

            case NORMAL_QUIET_MOVE : 
                                   {
                                    Relocate(p ,FromColor, From , To , FromPiece , FromPiece);
                                    break;
                                   }

            case DOUBLE_PAWN_PUSH :
                                   {
                                     Relocate(p ,FromColor, From , To , FromPiece , FromPiece);
                                     uint16_t En_Passant_Square = (FromColor == WHITE_OCCUPIED)?(To - 8):(To + 8);
                                     p.En_passant = (1ULL << En_Passant_Square);
                                     break;

                                   }

            case QUEEN_SIDE_CASTLE :
                                   {
                                    uint16_t KingToSquare   = From + 2;
                                    uint16_t RookFromSquare = To;
                                    uint16_t RookToSquare = KingToSquare - 1;

                                     Relocate(p , FromColor , From , KingToSquare , K , K);
                                     Relocate(p , FromColor , RookFromSquare, RookToSquare, R , R);


                                     p.Castle[(FromColor == WHITE_OCCUPIED)? wqside : bqside] = false;
                                     break;

                                   }

                                
            case KING_SIDE_CASTLE :
                                   {

                                    uint16_t KingToSquare   = From - 2;
                                    uint16_t RookFromSquare = To;
                                    uint16_t RookToSquare = KingToSquare + 1;

                                     Relocate(p , FromColor , From , KingToSquare , K , K);
                                     Relocate(p , FromColor , RookFromSquare, RookToSquare, R , R);

                                     p.Castle[(FromColor == WHITE_OCCUPIED)? wkside : bkside] = false;
                                     break;


                                   }

            case PROMOTE_TO_KNIGHT_QUIET :
                                   {

                                    Relocate(p , FromColor , From , To , P , N);
                                    break;

                                   }
            case PROMOTE_TO_BISHOP_QUIET :
                                   {
                                    Relocate(p , FromColor , From , To , P , B);
                                    break;
                                   }

            case PROMOTE_TO_ROOK_QUIET :
                                   {
                                    Relocate(p , FromColor , From , To , P , R);
                                    break;
                                   }
            case PROMOTE_TO_QUEEN_QUIET :
                                   {
                                    Relocate(p , FromColor , From , To , P , Q);
                                    break;

                                   }
            case EN_PASSANT  :
                                   {
                                      Relocate(p , FromColor , From , To , P , P);
                                      uint16_t En_Passant_Capture_Sq = (FromColor == WHITE_OCCUPIED)? (To - 8) : (To + 8);
                                     RemovePiece(p , P , En_Passant_Capture_Sq , (FromColor == WHITE_OCCUPIED)?BLACK_OCCUPIED : WHITE_OCCUPIED);
                                     break;

                                   }

            case NORMAL_CAPTURE  : 
                                    {
                                      Relocate(p , FromColor , From , To , FromPiece , FromPiece);
                                      RemovePiece(p , ToPiece , To , ToColor);
                                      break;
                                    }


                                

            case CAPTURE_AND_PROMOTE_TO_KNIGHT  :
                                   {

                                     Relocate(p , FromColor , From , To , P , N);
                                     RemovePiece(p , ToPiece , To , ToColor);
                                     break;
                                    
                                   }
            case CAPTURE_AND_PROMOTE_TO_BISHOP  :
                                   {

                                     Relocate(p , FromColor , From , To , P , B);
                                     RemovePiece(p , ToPiece , To , ToColor);
                                     break;


                                   }

            case CAPTURE_AND_PROMOTE_TO_ROOK  :
                                   {
                                     Relocate(p , FromColor , From , To , P , R);
                                     RemovePiece(p , ToPiece , To , ToColor);
                                     break;
                        
                                   }
            case CAPTURE_AND_PROMOTE_TO_QUEEN  :
                                   {
                                     Relocate(p , FromColor , From , To , P , Q);
                                     RemovePiece(p , ToPiece , To , ToColor);
                                     break;
                                   }
            
        }

        if(Flag != DOUBLE_PAWN_PUSH) p.En_passant = 0;
        p.ToMove = !p.ToMove;

       

        PrintBoard(p.Board);
        return p;




    }


    bool LeavesKingInCheck(Position &p , uint16_t Move){

        Position NewPosition = MakeMove(p , Move);

        std::span<uint64_t>Board(NewPosition.Board);
        

        bool BlackInCheck = IsBlackInCheck(Board);
        bool WhiteInCheck = IsWhiteInCheck(Board);
        

        if(NewPosition.ToMove == white && BlackInCheck) return true;
        if(NewPosition.ToMove == black && WhiteInCheck) return true;
        
        return false;

    }

};

int main()
{

    Board board1;

    board1.fenparsing("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");
    board1.PrintBoard(board1.position.Board);
    board1.GenerateLegalMoves(board1.position);
    board1.PrintHumanReadableMoveList();

    
}