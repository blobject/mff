using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public enum C { Black, White }

  public class Game
  {
    public Board Board { get; set; }
    public C Turn { get; set; }
    public List<string> Log { get; set; }
    private List<Piece> blackHand;
    private List<Piece> whiteHand;
    private List<Piece> blackCaptives;
    private List<Piece> whiteCaptives;

    public Game()
    {
      this.Board = new Board();
      this.blackCaptives = new List<Piece>();
      this.whiteCaptives = new List<Piece>();
      this.Turn = C.White;
      this.Log = new List<string>();
      this.blackHand = new List<Piece>();
      this.whiteHand = new List<Piece>();
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 7; rank <= 8; rank++)
        {
          this.blackHand.Add(this.Board.At(new Place(file, rank)));
        }
      }
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 2; rank >= 1; rank--)
        {
          this.whiteHand.Add(this.Board.At(new Place(file, rank)));
        }
      }
    }

    // for testing
    public Game(string strands) // "pieces (of) string"
    {
      List<Piece> blackPieces;
      List<Piece> whitePieces;
      this.Board = new Board(strands, out blackPieces, out whitePieces);
      this.blackHand = blackPieces;
      this.whiteHand = whitePieces;
      this.blackCaptives = new List<Piece>();
      this.whiteCaptives = new List<Piece>();
      this.Turn = C.White;
      this.Log = new List<string>();
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
    Other()
    {
      if (this.Turn == C.Black)
      {
        return C.White;
      }
      return C.Black;
    }

    public Piece
    At(Place place)
    {
      return this.Board.At(place);
    }

    public void
    Set(Place place, Piece piece)
    {
      this.Board.Set(place, piece);
    }

    public void
    Clear(Place place)
    {
      Piece piece = At(place);
      this.blackHand.Remove(piece);
      this.whiteHand.Remove(piece);
      this.blackCaptives.Remove(piece);
      this.whiteCaptives.Remove(piece);
      this.Board.Clear(place);
    }

    public void
    ClearAll()
    {
      this.blackHand.Clear();
      this.whiteHand.Clear();
      this.blackCaptives.Clear();
      this.whiteCaptives.Clear();
      this.Board.ClearAll();
    }

    // for testing
    public void
    Put(string strands)
    {
      List<Piece> blackPieces;
      List<Piece> whitePieces;
      this.Board.Put(strands, out blackPieces, out whitePieces);
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
      if (this.Turn == C.Black)
      {
        this.Turn = C.White;
        return;
      }
      this.Turn = C.Black;
    }
  }
}
