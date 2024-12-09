#include <iostream>
//We include necessary STL libraries such as map, set, stack, algorithm and tuple for handling game pieces, moves, and game logic.
#include <map>       
// Used for efficiently storing and accessing chess pieces at specific board positions.
#include <set>       
// Used to track unique legal moves, captured pieces, and other sets that require ordered uniqueness.
#include <list>      
// Used to store the history of moves made, in chronological order.
#include <stack>     
// Used for storing the history of moves so that the game can be undone.
#include <string>    
// Used for handling textual information, such as player names, move descriptions, and other string-based data.
#include <algorithm> 
// Used to perform sorting, searching, and other operations on STL containers.
#include <tuple> 
// Used to store and return multiple values from a function.
#include <unordered_map> 
// Used to store and access chess pieces at specific board positions.

enum class Color { WHITE, BLACK }; 
// Enum class to represent the color of a chess piece.

class Position { 
public: 
    char column;  
    int row;     

// Position class to represent a chess board position.
    Position(char c, int r) : column(c), row(r) {}

    bool operator==(const Position& other) const { 
        // Overloaded equality operator to compare two Position objects.
        return column == other.column && row == other.row;
    }
// This function is used to compare two Position objects to determine if they represent the same position on the chess board.

    bool operator<(const Position& other) const {
        return std::tie(column, row) < std::tie(other.column, other.row);
    }
// This function is used to compare two Position objects to determine their relative order.
};

namespace std {
    template<> 
    // Template specialization for hashing Position objects.
    struct hash<Position> { 
        std::size_t operator()(const Position& pos) const { 
          // Overloaded hash function for Position objects.
            return static_cast<std::size_t>(pos.column) * 10 + pos.row; 
        }
    };
}
// This function is used to hash Position objects to create a unique hash value for each position on the chess board.

// Abstract base class representing a generic chess piece.
class ChessPiece {
protected:
    Color color;            // The color of the piece (either WHITE or BLACK).
    Position position;      // The current position of the piece on the chessboard.
    bool hasMoved;          // Tracks whether the piece has moved (important for rules like castling or en passant).

public:
    // Constructor to initialize a chess piece with its color and position.
    // By default, 'hasMoved' is set to false because the piece has not moved initially.
    ChessPiece(Color c, Position p) : color(c), position(p), hasMoved(false) {}

    // Virtual destructor to ensure proper cleanup of derived classes when a ChessPiece pointer is deleted.
    virtual ~ChessPiece() {}

    // Getter method to retrieve the color of the chess piece (WHITE or BLACK).
    Color getColor() const { return color; }

    // Getter method to retrieve the current position of the chess piece.
    Position getPosition() const { return position; }

    // Setter method to update the position of the chess piece to a new position.
    void setPosition(Position p) { position = p; }

    // Method to check if the piece has moved before (returns the value of 'hasMoved').
    bool hasMovedBefore() const { return hasMoved; }

    // Method to mark the piece as having moved. Sets 'hasMoved' to true.
    void markAsMoved() { hasMoved = true; }

    // Pure virtual method to calculate the legal moves for the chess piece.
    // This must be implemented by all derived classes (e.g., Pawn, Rook, Knight).
    // - 'board' represents the current state of the chessboard as an unordered_map,
    //    mapping positions to chess pieces.
    // - 'lastMovePos' stores the position of the last move (important for rules like en passant).
    // - 'enPassantAvailable' is a flag indicating whether an en passant move is available.
    virtual std::set<Position> legalMoves(
        const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board,
        const Position& lastMovePos,
        bool enPassantAvailable
    ) const = 0;

    // Pure virtual method to get a string representation of the piece.
    // This will return a single-character string (e.g., "P" for white pawn, "r" for black rook).
    virtual std::string toString() const = 0;
};

class Pawn : public ChessPiece {
public:
    Pawn(Color c, Position p) : ChessPiece(c, p) {}

    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, 
                                  const Position& lastMovePos, 
                                  bool enPassantAvailable) const override {
        std::set<Position> moves;

        int direction = (color == Color::WHITE) ? 1 : -1;

        Position oneStep(position.column, position.row + direction);
        if (board.find(oneStep) == board.end()) { 
            // No piece in the way
            moves.insert(oneStep);

            if (!hasMovedBefore()) {
                Position twoStep(position.column, position.row + 2 * direction);
                if (board.find(twoStep) == board.end()) {  
                    // No piece in the way
                    moves.insert(twoStep);
                }
            }
        }

        for (int dc = -1; dc <= 1; dc += 2) { 
            // Looping for left and right diagonal positions
            Position capturePos(position.column + dc, position.row + direction);
            if (board.find(capturePos) != board.end() && board.at(capturePos)->getColor() != color) {
                moves.insert(capturePos); 
                // Opponent's piece found, valid capture move
            }
        }

        // Check for en passant capture
        if (enPassantAvailable && !hasMovedBefore()) {
            // Check left and right squares
            Position leftEnPassant(position.column - 1, position.row);
            Position rightEnPassant(position.column + 1, position.row);
            if (lastMovePos == leftEnPassant || lastMovePos == rightEnPassant) {
                ChessPiece* adjacentPawn = board.at(lastMovePos);
                if (adjacentPawn && adjacentPawn->toString() == (color == Color::WHITE ? "p" : "P") && !adjacentPawn->hasMovedBefore()) {
                    Position enPassantPos(lastMovePos.column, position.row + direction);
                    moves.insert(enPassantPos);
                }
            }
        }

        return moves;  // Return all valid legal moves for this pawn
    }

    std::string toString() const override {
        return color == Color::WHITE ? "P" : "p";
    }
};

class Rook : public ChessPiece {
public:

    Rook(Color c, Position p) : ChessPiece(c, p) {}

    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, const Position& lastMovePos, bool enPassantAvailable) const override {
        std::set<Position> moves;

        for (int i = 1; i <= 8; ++i) {
            if (i != position.row) moves.insert(Position(position.column, i));
        }

        for (int i = 1; i <= 8; ++i) {
            if (i != position.column - 'a' + 1) moves.insert(Position('a' + i - 1, position.row));
        }

        return moves; 
        // Return all valid legal moves for this rook.
    }

    std::string toString() const override {
        return color == Color::WHITE ? "R" : "r"; 
        // Return 'R' for white rook, 'r' for black rook.
    }
};

class Knight : public ChessPiece {
public:
    Knight(Color c, Position p) : ChessPiece(c, p) {}

    // Calculates the legal moves of a knight. A knight moves in an "L" shape: two squares in one direction and then one square perpendicular.
    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, const Position& lastMovePos, bool enPassantAvailable) const override {
        std::set<Position> moves;

        const int dx[] = { 2, 2, -2, -2, 1, 1, -1, -1 };  
        // Horizontal moves (2 steps forward/backward and 1 step left/right)
        const int dy[] = { 1, -1, 1, -1, 2, -2, 2, -2 }; 
        // Vertical moves (1 step up/down and 2 steps left/right)

        for (int i = 0; i < 8; ++i) {
            // Calculate the new row and column after applying the move.
            int newRow = position.row + dx[i];  
            // Add the horizontal movement
            int newCol = position.column + dy[i]; 
            // Add the vertical movement

            // Check if the new position is within the bounds of the chessboard (1-8 for rows and 'a'-'h' for columns).
            if (newRow >= 1 && newRow <= 8 && newCol >= 'a' && newCol <= 'h') {
                // If the move is within bounds, add it to the set of possible moves.
                moves.insert(Position(newCol, newRow));
            }
        }

        return moves;  
        // Return all valid legal moves for this knight.
    }

    std::string toString() const override {
        return color == Color::WHITE ? "N" : "n";  
        // Return 'N' for white knight, 'n' for black knight.
    }
};

class Bishop : public ChessPiece {
public:

    Bishop(Color c, Position p) : ChessPiece(c, p) {}

    // Calculates the legal moves of a bishop. A bishop can move diagonally in any direction.
    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, const Position& lastMovePos, bool enPassantAvailable) const override {
        std::set<Position> moves;

        // Diagonal movement (Corrected logic)
        for (int i = 1; i <= 8; ++i) {
            // Top-Right Diagonal
            Position topRight(position.column + i, position.row + i);
            if (topRight.row >= 1 && topRight.row <= 8 && topRight.column >= 'a' && topRight.column <= 'h') {
                if (board.find(topRight) == board.end() || board.at(topRight)->getColor() != color) {
                    moves.insert(topRight);
                    if (board.find(topRight) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Top-Left Diagonal
            Position topLeft(position.column - i, position.row + i);
            if (topLeft.row >= 1 && topLeft.row <= 8 && topLeft.column >= 'a' && topLeft.column <= 'h') {
                if (board.find(topLeft) == board.end() || board.at(topLeft)->getColor() != color) {
                    moves.insert(topLeft);
                    if (board.find(topLeft) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Bottom-Right Diagonal
            Position bottomRight(position.column + i, position.row - i);
            if (bottomRight.row >= 1 && bottomRight.row <= 8 && bottomRight.column >= 'a' && bottomRight.column <= 'h') {
                if (board.find(bottomRight) == board.end() || board.at(bottomRight)->getColor() != color) {
                    moves.insert(bottomRight);
                    if (board.find(bottomRight) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Bottom-Left Diagonal
            Position bottomLeft(position.column - i, position.row - i);
            if (bottomLeft.row >= 1 && bottomLeft.row <= 8 && bottomLeft.column >= 'a' && bottomLeft.column <= 'h') {
                if (board.find(bottomLeft) == board.end() || board.at(bottomLeft)->getColor() != color) {
                    moves.insert(bottomLeft);
                    if (board.find(bottomLeft) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }
        }

        return moves; 
    }

    std::string toString() const override {
        return color == Color::WHITE ? "B" : "b"; 
        // Return 'B' for white bishop, 'b' for black bishop.
    }
};

class Queen : public ChessPiece {
public:
    // Constructor initializes the Queen with a color and position.
    Queen(Color c, Position p) : ChessPiece(c, p) {}

    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, const Position& lastMovePos, bool enPassantAvailable) const override {
        std::set<Position> moves;
        for (int i = 1; i <= 8; ++i) {
            if (i != position.row) moves.insert(Position(position.column, i)); 
            // Adds vertical moves.
            if (i != position.column - 'a' + 1) moves.insert(Position('a' + i - 1, position.row));  
            // Adds horizontal moves.
        }

        // Diagonal movement (Corrected logic)
        for (int i = 1; i <= 8; ++i) {
            // Top-Right Diagonal
            Position topRight(position.column + i, position.row + i);
            if (topRight.row >= 1 && topRight.row <= 8 && topRight.column >= 'a' && topRight.column <= 'h') {
                if (board.find(topRight) == board.end() || board.at(topRight)->getColor() != color) {
                    moves.insert(topRight);
                    if (board.find(topRight) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Top-Left Diagonal
            Position topLeft(position.column - i, position.row + i);
            if (topLeft.row >= 1 && topLeft.row <= 8 && topLeft.column >= 'a' && topLeft.column <= 'h') {
                if (board.find(topLeft) == board.end() || board.at(topLeft)->getColor() != color) {
                    moves.insert(topLeft);
                    if (board.find(topLeft) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Bottom-Right Diagonal
            Position bottomRight(position.column + i, position.row - i);
            if (bottomRight.row >= 1 && bottomRight.row <= 8 && bottomRight.column >= 'a' && bottomRight.column <= 'h') {
                if (board.find(bottomRight) == board.end() || board.at(bottomRight)->getColor() != color) {
                    moves.insert(bottomRight);
                    if (board.find(bottomRight) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }

            // Bottom-Left Diagonal
            Position bottomLeft(position.column - i, position.row - i);
            if (bottomLeft.row >= 1 && bottomLeft.row <= 8 && bottomLeft.column >= 'a' && bottomLeft.column <= 'h') {
                if (board.find(bottomLeft) == board.end() || board.at(bottomLeft)->getColor() != color) {
                    moves.insert(bottomLeft);
                    if (board.find(bottomLeft) != board.end()) {
                        break; // Stop if we encounter a piece (own or opponent's)
                    }
                }
            }
        }

        return moves;  
        // Returns the legal moves for the queen (incomplete for full movement logic).
    }

    std::string toString() const override {
        return color == Color::WHITE ? "Q" : "q";  // Return 'Q' for white queen, 'q' for black queen.
    }
};

class King : public ChessPiece {
public:
    // Constructor initializes the King with a color and position.
    King(Color c, Position p) : ChessPiece(c, p) {}

    std::set<Position> legalMoves(const std::unordered_map<Position, ChessPiece*, std::hash<Position>>& board, const Position& lastMovePos, bool enPassantAvailable) const override {
        std::set<Position> moves;

        for (int dx = -1; dx <= 1; ++dx) {  
            // Loop through x-direction (horizontal movement)
            for (int dy = -1; dy <= 1; ++dy) {  
                // Loop through y-direction (vertical movement)
                if (dx == 0 && dy == 0) continue;  
                // Skip the current position (no move)

                // Calculate the new position the king could move to
                Position newPos(position.column + dx, position.row + dy);

                if (newPos.row >= 1 && newPos.row <= 8 && newPos.column >= 'a' && newPos.column <= 'h') {
                    // Check if the new position is either empty or occupied by an opponent's piece
                    if (board.find(newPos) == board.end() || board.at(newPos)->getColor() != color) {
                        moves.insert(newPos);  
                        // Add valid move to the set of possible moves
                    }
                }
            }
        }

        return moves; 
        // Return the set of legal moves for the king.
    }

    std::string toString() const override {
        return color == Color::WHITE ? "K" : "k";  
        // Return 'K' for white king, 'k' for black king.
    }
};

class Board {
private:
    std::unordered_map<Position, ChessPiece*, std::hash<Position>> pieceMap; 
    //A std::map that associates positions with pieces on the board.
    Color turn; 
    //The color of the player who is currently to move.
    Position lastMovePos;
    //The position of the last move made.
    bool enPassantAvailable;
    //A flag indicating if en passant is available for the current player.
    bool whiteCastleKingSide, whiteCastleQueenSide; 
    //Flags indicating if white castling is available.
    bool blackCastleKingSide, blackCastleQueenSide; 
    //Flags indicating if black castling is available.

public:
    Board() : lastMovePos(Position('a', 1)), enPassantAvailable(false), 
      whiteCastleKingSide(true), whiteCastleQueenSide(true),
      blackCastleKingSide(true), blackCastleQueenSide(true) {
        // Initialize the board with the starting pieces.
        pieceMap = std::unordered_map<Position, ChessPiece*, std::hash<Position>>(); 
        // Initialize pieceMap here. 
        initialize();
        turn = Color::WHITE;
    }

~Board() {
    for (auto& entry : pieceMap) {
        delete entry.second;
        // Deallocate memory for each piece
    }
}

    void initialize();
    void display() const;
    bool movePiece(const std::string& from, const std::string& to);
    std::string getTurnName() const { return turn == Color::WHITE ? "White" : "Black"; } 
    // Method to get the turn name
    void updateLastMove(Position pos) {
        lastMovePos = pos;
    }

    Position getLastMove() const {
        return lastMovePos;
    } 
    bool canCastleKingSide(Color color) const;
    bool canCastleQueenSide(Color color) const;
    void updateCastlingRights(Color color, const std::string& pieceType);
    bool isInCheck(Color color);
    bool isCheckmate(Color color);
    bool isStalemate(Color color);

    bool simulateMoveAndCheck(const Position& from, const Position& to, Color color);

    ChessPiece* findKing(Color color) const;

    Color getTurn() const { return turn; } 
    // Added getter method for 'turn'
    bool isEnPassantAvailable() const { return enPassantAvailable; }
};

// Method to initialize the chessboard with the starting positions of all pieces.
// This method sets up both White and Black pieces in their respective starting positions.
void Board::initialize() {
    // Place White's major pieces on the first rank (row 1).
    pieceMap[Position('a', 1)] = new Rook(Color::WHITE, Position('a', 1));    
    pieceMap[Position('b', 1)] = new Knight(Color::WHITE, Position('b', 1));  
    pieceMap[Position('c', 1)] = new Bishop(Color::WHITE, Position('c', 1));  
    pieceMap[Position('d', 1)] = new Queen(Color::WHITE, Position('d', 1));  
    pieceMap[Position('e', 1)] = new King(Color::WHITE, Position('e', 1));   
    pieceMap[Position('f', 1)] = new Bishop(Color::WHITE, Position('f', 1)); 
    pieceMap[Position('g', 1)] = new Knight(Color::WHITE, Position('g', 1)); 
    pieceMap[Position('h', 1)] = new Rook(Color::WHITE, Position('h', 1));    

    // Place White's pawns on the second rank (row 2).
    for (char col = 'a'; col <= 'h'; ++col) {
        pieceMap[Position(col, 2)] = new Pawn(Color::WHITE, Position(col, 2)); // White Pawn on row 2
    }

    // Place Black's major pieces on the eighth rank (row 8).
    pieceMap[Position('a', 8)] = new Rook(Color::BLACK, Position('a', 8));    
    pieceMap[Position('b', 8)] = new Knight(Color::BLACK, Position('b', 8));  
    pieceMap[Position('c', 8)] = new Bishop(Color::BLACK, Position('c', 8));  
    pieceMap[Position('d', 8)] = new Queen(Color::BLACK, Position('d', 8));   
    pieceMap[Position('e', 8)] = new King(Color::BLACK, Position('e', 8));    
    pieceMap[Position('f', 8)] = new Bishop(Color::BLACK, Position('f', 8));  
    pieceMap[Position('g', 8)] = new Knight(Color::BLACK, Position('g', 8)); 
    pieceMap[Position('h', 8)] = new Rook(Color::BLACK, Position('h', 8));  

    // Place Black's pawns on the seventh rank (row 7).
    for (char col = 'a'; col <= 'h'; ++col) {
        pieceMap[Position(col, 7)] = new Pawn(Color::BLACK, Position(col, 7)); // Black Pawn on row 7
    }
}

void Board::display() const {
    std::cout << "  a b c d e f g h\n";
    for (int row = 8; row >= 1; --row) {
        std::cout << row << " ";
        for (char col = 'a'; col <= 'h'; ++col) {
            Position pos(col, row);
            if (pieceMap.find(pos) != pieceMap.end()) {
                std::cout << pieceMap.at(pos)->toString() << " ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << row << std::endl;
    }
    std::cout << "  a b c d e f g h\n";
}

bool Board::movePiece(const std::string& from, const std::string& to) {
    // Convert the string positions ('e2' to Position('e', 2)) for both from and to.
    Position fromPos(from[0], from[1] - '0');
    Position toPos(to[0], to[1] - '0');

    // Check if the piece at the 'from' position exists.
    if (pieceMap.find(fromPos) == pieceMap.end()) {
        return false; // No piece found at 'from', return false.
    }

    // Get the piece at the 'from' position.
    ChessPiece* piece = pieceMap[fromPos];

    // Check if the piece is a Pawn.
    bool isPawn = dynamic_cast<Pawn*>(piece) != nullptr;

    // Check if the piece is a King (by comparing the piece type string).
    bool isKing = piece->toString() == (piece->getColor() == Color::WHITE ? "K" : "k");

    bool enPassantCapture = false;
    if (isPawn) {
        // If the piece is a Pawn, check if the move is legal (including en passant).
        Pawn* pawn = dynamic_cast<Pawn*>(piece);
        auto moves = pawn->legalMoves(pieceMap, lastMovePos, enPassantAvailable);
        if (moves.find(toPos) == moves.end()) {
            return false; // Move is not legal for this pawn, return false.
        }
        enPassantCapture = enPassantAvailable && (lastMovePos == Position(toPos.column, fromPos.row)); // Check for en passant capture.
        // Update enPassantAvailable if a pawn moves two squares forward
        if (std::abs(toPos.row - fromPos.row) == 2) {
            enPassantAvailable = true; 
        } else {
            enPassantAvailable = false;
        }
    } 

    // If the piece is a King, check if castling is possible (either King-side or Queen-side).
    else if (isKing && canCastleKingSide(piece->getColor()) && toPos == Position('g', fromPos.row)) {
        Position rookPos('h', fromPos.row);
        ChessPiece* rook = pieceMap[rookPos];
        if (rook && rook->toString() == (piece->getColor() == Color::WHITE ? "R" : "r")) {
            // Perform castling by moving the King and Rook.
            Position newKingPos('f', fromPos.row);
            piece->setPosition(newKingPos);
            rook->setPosition(Position('e', fromPos.row));
            updateCastlingRights(piece->getColor(), "king");
        }
    } 

    else if (isKing && canCastleQueenSide(piece->getColor()) && toPos == Position('c', fromPos.row)) {
        Position rookPos('a', fromPos.row);
        ChessPiece* rook = pieceMap[rookPos];
        if (rook && rook->toString() == (piece->getColor() == Color::WHITE ? "R" : "r")) {
            // Perform castling by moving the King and Rook.
            Position newKingPos('d', fromPos.row);
            piece->setPosition(newKingPos);
            rook->setPosition(Position('e', fromPos.row));
            updateCastlingRights(piece->getColor(), "queen");
        }
    }

    // Handle en passant capture if applicable.
    if (enPassantCapture) {
        Position capturedPawnPos(toPos.column, fromPos.row);
        delete pieceMap[capturedPawnPos]; // Capture the pawn via en passant.
        pieceMap.erase(capturedPawnPos);
    }

    // Finalize the move: update piece position and mark it as moved.
    piece->setPosition(toPos);
    piece->markAsMoved();
    pieceMap[toPos] = piece;
    pieceMap.erase(fromPos); // Remove the piece from the original position.

    // Update the last move, reset en passant availability, and switch turns.
    lastMovePos = toPos;
    enPassantAvailable = false; // Reset en passant availability after each move
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    return true; // Successfully completed the move.
}

// Method to check if castling on the King-side (short castling) is possible for a given color.
// Parameters:
// - color: The color of the player (WHITE or BLACK).
// Returns:
// - true if the King-side castling is still allowed, false otherwise.
bool Board::canCastleKingSide(Color color) const {
    if (color == Color::WHITE) { // Check if the player is White.
        return whiteCastleKingSide; // Return White's King-side castling status.
    }
    return blackCastleKingSide; // Return Black's King-side castling status.
}

bool Board::canCastleQueenSide(Color color) const {
    if (color == Color::WHITE) { // Check if the player is White.
        return whiteCastleQueenSide; // Return White's Queen-side castling status.
    }
    return blackCastleQueenSide; // Return Black's Queen-side castling status.
}

// Castling rights are removed when the king or the relevant rook moves.
// Parameters:
// - color: The color of the player (WHITE or BLACK).
// - pieceType: The type of piece that moved, expected to be "king" or "rook".
void Board::updateCastlingRights(Color color, const std::string& pieceType) {
    if (color == Color::WHITE) { // Check if the piece belongs to the White player.
        // Update White's castling rights.
        if (pieceType == "king") {
            // If the White King has moved, both King-side and Queen-side castling are no longer allowed.
            whiteCastleKingSide = false;
            whiteCastleQueenSide = false;
        }
        // Note: Logic for updating castling rights for rooks would go here if needed.
    } 
    else { // The piece belongs to the Black player.
        // Update Black's castling rights.
        if (pieceType == "king") {
            // If the Black King has moved, both King-side and Queen-side castling are no longer allowed.
            blackCastleKingSide = false;
            blackCastleQueenSide = false;
        }
    }
}

// Function to check if a king is in check
bool Board::isInCheck(Color color) { 
  // Function to check if a king is in check
    Position kingPos = findKing(color)->getPosition(); 
  // Find the king's position
    for (const auto& entry : pieceMap) {
        if (entry.second->getColor() != color) {
            auto moves = entry.second->legalMoves(pieceMap, lastMovePos, enPassantAvailable); 
            // Pass enPassantAvailable
            if (moves.find(kingPos) != moves.end()) {
                return true;
            }
        }
    }

    return false;
}

// Function to check if the current player is in checkmate
bool Board::isCheckmate(Color color) {
    if (!isInCheck(color)) return false;  
   // If the current player is not in check, it's not checkmate

    for (const auto& entry : pieceMap) {
        if (entry.second->getColor() == color) {  
          // Iterate over pieces of the current player
            auto moves = entry.second->legalMoves(pieceMap, lastMovePos, enPassantAvailable); 
            // Pass enPassantAvailable
            for (const auto& move : moves) {
                if (!simulateMoveAndCheck(entry.first, move, color)) {
                  // Check if the move is legal

                    return false;
                }
            }
        }
    }
    return true;
}

// Check if the current player is in stalemate
bool Board::isStalemate(Color color) { 

    if (isInCheck(color)) return false;  
  // Check if the king is in check

    for (const auto& entry : pieceMap) {
        if (entry.second->getColor() == color) { 
          // Check if the piece is of the given color

            auto moves = entry.second->legalMoves(pieceMap, lastMovePos, enPassantAvailable);
            // Pass enPassantAvailable
            for (const auto& move : moves) { 
              // Loop through all possible moves

                if (!simulateMoveAndCheck(entry.first, move, color)) {
                  // Check if the move is valid
                    return false;
                }
            }
        }
    }
    return true;
}

bool Board::simulateMoveAndCheck(const Position& from, const Position& to, Color color) {
    ChessPiece* piece = pieceMap[from];
    ChessPiece* capturedPiece = pieceMap[to]; 
    // Store the piece at the target position

    // Simulate the move
    piece->setPosition(to);
    // Move the piece to the target position
    pieceMap[to] = piece; 
    // Update the board map
    pieceMap.erase(from);
    // Remove the piece from its original position

    bool inCheck = isInCheck(color);

    // Revert the move
    piece->setPosition(from);
    // Reset the piece's position
    pieceMap[from] = piece;
    // Update the board map
    pieceMap[to] = capturedPiece;
    // Reset the piece at the target position

    // Clean up
    return inCheck;
}


ChessPiece* Board::findKing(Color color) const {
    for (const auto& entry : pieceMap) {
        if (entry.second->getColor() == color && entry.second->toString() == (color == Color::WHITE ? "K" : "k")) {
            // King found
            return entry.second;
        }
    }

    return nullptr;
}

//This is the main game loop, where the board is displayed, and the user is prompted for input. The loop continues until the program is terminated.
int main() {
    std::cout << "Welcome to My Chess Game!\n";
    std::cout << "In this game, you will move pieces on a chessboard to checkmate your opponent.\n";
    std::cout << "Each player takes turns moving one piece at a time.\n";
    std::cout << "Type your move using standard chess notation (e.g., 'e2 e4').\n";
    std::cout << "You can move a piece to an empty square or capture an opponent's piece by moving to its square.\n\n";
    std::cout << "Let's get started!\n\n";


    Board board;
    board.display();  

    std::string from, to;
    while (true) {
        // Display the current player's turn
        std::cout << "It's " << board.getTurnName() << "'s turn.\n";
        std::cout << "Enter your move in the format 'from_square to_square' (e.g., 'e2 e4').\n";
        std::cout << "Example: Move your pawn from 'e2' to 'e4'.\n";
        std::cin >> from >> to;

        if (board.movePiece(from, to)) {
            board.display();

// Check if the game is over
            if (board.isCheckmate(board.getTurn())) { // Using the getter method
                std::cout << "\nCheckmate! " << board.getTurnName() << " is in checkmate! Game over.\n";
                std::cout << "Congratulations to the winner, " << board.getTurnName() << "!\n";
                std::cout << "Would you like to play again? (Y/N)\n";
                break;
              // Ask the user if they want to play again
            } else if (board.isStalemate(board.getTurn())) { // Using the getter method
                std::cout << "\nIt's a Stalemate! Neither player can make a legal move. Game over.\n";
                std::cout << "No winner this time. Better luck next time!\n";
                break;
            }

        } else {
            std::cout << "\nOops! That move is invalid.\n";
            std::cout << "Make sure you are following these rules:\n";
            std::cout << "- A piece can only move according to its allowed pattern.\n";
            std::cout << "- You cannot move a piece to a square already occupied by a piece of the same color.\n";
            std::cout << "- Ensure that your move uses the correct chess notation (e.g., 'e2 e4').\n";
            std::cout << "Try again.\n";
        }
    }

    std::cout << "\nThank you for playing! We hope you enjoyed the game.\n";
    std::cout << "Feel free to play again anytime!\n";

    return 0;
}