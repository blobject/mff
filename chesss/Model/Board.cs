using System;
using System.Collections.Generic;
using Chesss.Util;

namespace Chesss.Model
{
  public class Board
  {
    private List<List<Piece>> squares;

    public Board()
    {
      this.squares = new List<List<Piece>>();
      P sym = P.Rook;

      for (var file = 1; file <= 8; file++)
      {
        if      (file == 1 || file == 8) { sym = P.Rook; }
        else if (file == 2 || file == 7) { sym = P.Knight; }
        else if (file == 3 || file == 6) { sym = P.Bishop; }
        else if (file == 4)              { sym = P.Queen; }
        else if (file == 5)              { sym = P.King; }
        var col = new List<Piece>();
        col.Add(new Piece(sym, C.White, new Place(file, 1)));
        col.Add(new Piece(P.Pawn, C.White, new Place(file, 2)));
        for (int rank = 3; rank <= 6; rank++)
        {
          col.Add(null);
        }
        col.Add(new Piece(P.Pawn, C.Black, new Place(file, 7)));
        col.Add(new Piece(sym, C.Black, new Place(file, 8)));
        this.squares.Add(col);
      }
    }

    // for testing
    public Board(string strands,
                 out List<Piece> blackPieces,
                 out List<Piece> whitePieces)
    {
      blackPieces = new List<Piece>();
      whitePieces = new List<Piece>();
      List<Piece> pieces;
      int ret = Helper.ParsePieces(strands, out pieces);
      if (ret != 0)
      {
        Console.WriteLine("Error parsing strands");
        return;
      }

      this.squares = new List<List<Piece>>();
      for (int file = 1; file <= 8; file++)
      {
        var col = new List<Piece>();
        for (int rank = 1; rank <= 8; rank++)
        {
          col.Add(null);
        }
        squares.Add(col);
      }

      foreach (var piece in pieces)
      {
        if (piece.Color() == C.Black)
        {
          blackPieces.Add(piece);
        }
        else
        {
          whitePieces.Add(piece);
        }
        Set(piece.Place(), piece);
      }
    }

    public Piece
    At(Place place)
    {
      return this.squares[place.File() - 1][place.Rank() - 1];
    }

    public void
    Set(Place place, Piece piece)
    {
      this.squares[place.File() - 1][place.Rank() - 1] = piece;
    }

    public void
    Clear(Place place)
    {
      Set(place, null);
    }

    // for testing
    public void
    ClearAll()
    {
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 1; rank <= 8; rank++)
        {
          Clear(new Place(file, rank));
        }
      }
    }

    // for testing
    public void
    Put(string strands,
        out List<Piece> blackPieces,
        out List<Piece> whitePieces)
    {
      blackPieces = new List<Piece>();
      whitePieces = new List<Piece>();
      List<Piece> pieces;
      int ret = Helper.ParsePieces(strands, out pieces);
      if (ret != 0)
      {
        Console.WriteLine("Error parsing pieces-string");
        return;
      }

      foreach (var piece in pieces)
      {
        if (At(piece.Place()) != null)
        {
          Console.WriteLine("Not putting piece, {0} already occupied",
                            piece.Place());
          continue;
        }
        if (piece.Color() == C.Black)
        {
          blackPieces.Add(piece);
        }
        else
        {
          whitePieces.Add(piece);
        }
        Set(piece.Place(), piece);
      }
    }
  }
}
