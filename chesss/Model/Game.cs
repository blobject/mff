using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public enum C
  {
    Black,
    White
  }

  public enum P
  {
    Pawn,
    King,
    Queen,
    Bishop,
    Knight,
    Rook
  }

  public class Game
  {
    private Board board;
    private List<Piece> blackHand;
    private List<Piece> whiteHand;
    private List<Piece> blackCaptives;
    private List<Piece> whiteCaptives;
    private C turn;
    private int turns;

    public Game()
    {
      this.board = new Board();
      this.blackCaptives = new List<Piece>();
      this.whiteCaptives = new List<Piece>();
      this.turn = C.White;
      this.turns = 0;
      this.blackHand = new List<Piece>();
      this.whiteHand = new List<Piece>();
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 7; rank <= 8; rank++)
        {
          blackHand.Add(board.At(new Place(file, rank)));
        }
      }
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 2; rank <= 1; rank--)
        {
          whiteHand.Add(board.At(new Place(file, rank)));
        }
      }
    }

    // for testing
    public Game(string strands) // "pieces (of) string"
    {
      List<Piece> blackPieces;
      List<Piece> whitePieces;
      this.board = new Board(strands, out blackPieces, out whitePieces);
      this.blackHand = blackPieces;
      this.whiteHand = whitePieces;
      this.blackCaptives = new List<Piece>();
      this.whiteCaptives = new List<Piece>();
      this.turn = C.White;
      this.turns = 0;
    }

    public Board
    Board()
    {
      return this.board;
    }

    public List<Piece>
    Hand(C color)
    {
      if (color == C.Black)
      {
        return this.blackHand;
      }
      return this.whiteHand;
    }

    public List<Piece>
    Captives(C color)
    {
      if (color == C.Black)
      {
        return this.blackCaptives;
      }
      return this.whiteCaptives;
    }

    public C
    Turn()
    {
      return this.turn;
    }

    public C
    Other()
    {
      if (this.turn == C.Black)
      {
        return C.White;
      }
      return C.Black;
    }

    public int
    Turns()
    {
      return this.turns;
    }

    public Piece
    At(Place place)
    {
      return this.board.At(place);
    }

    public void
    Set(Place place, Piece piece)
    {
      this.board.Set(place, piece);
    }

    public void
    Clear(Place place)
    {
      Piece piece = At(place);
      blackHand.Remove(piece);
      whiteHand.Remove(piece);
      blackCaptives.Remove(piece);
      whiteCaptives.Remove(piece);
      this.board.Clear(place);
    }

    public void
    ClearAll()
    {
      blackHand.Clear();
      whiteHand.Clear();
      blackCaptives.Clear();
      whiteCaptives.Clear();
      this.board.ClearAll();
    }

    // for testing
    public void
    Put(string strands)
    {
      List<Piece> blackPieces;
      List<Piece> whitePieces;
      this.board.Put(strands, out blackPieces, out whitePieces);
      foreach (var piece in blackPieces)
      {
        this.blackHand.Add(piece);
      }
      foreach (var piece in whitePieces)
      {
        this.whiteHand.Add(piece);
      }
    }

    public void
    Toggle()
    {
      if (this.turn == C.Black)
      {
        this.turn = C.White;
        return;
      }
      this.turn = C.Black;
      this.turns++;
    }
  }
}
