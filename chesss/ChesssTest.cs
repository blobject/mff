// EXECUTION:
// mono-csc -target:library \
//          -reference:../NUnit.2.6.4/lib/nunit.framework.dll \
//          -reference:./Chesss.exe ./ChesssTest.cs \
// && MONO_PATH=../NUnit.2.6.4/lib nunit-console ./ChesssTest.dll

using System;
using System.Collections.Generic;
using NUnit.Framework;
using Chesss.Controller;
using Chesss.Model;
using Chesss.Util;

namespace Chesss.Test
{
  [TestFixture]
  public class RunModelTests
  {
    [Test]
    public void BoardInit()
    {
      Game game = new Game();
      Place place;
      Piece piece;

      Assert.AreEqual(C.White, game.Turn());
      Assert.AreEqual(0, game.Turns());

      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 1; rank <= 8; rank++)
        {
          place = new Place(file, rank);
          piece = game.At(place);
          // empty squares
          if (rank > 2 && rank < 7)
          {
            Assert.AreEqual(null, piece);
            continue;
          }
          // white pieces
          if (rank <= 2)
          {
            Assert.AreEqual(C.White, piece.Color());
          }
          // black pieces
          else if (rank >= 7)
          {
            Assert.AreEqual(C.Black, piece.Color());
          }
          // commonalities
          Assert.True(piece.Inert());
          Assert.AreEqual(place, piece.Place());
          if (rank == 2 || rank == 7)
          {
            Assert.AreEqual(P.Pawn, piece.Sym());
          }
          else if (rank == 1 || rank == 8)
          {
            if (file == 1 || file == 8)
            { Assert.AreEqual(P.Rook, piece.Sym()); }
            else if (file == 2 || file == 7)
            { Assert.AreEqual(P.Knight, piece.Sym()); }
            else if (file == 3 || file == 6)
            { Assert.AreEqual(P.Bishop, piece.Sym()); }
            else if (file == 4)
            { Assert.AreEqual(P.Queen, piece.Sym()); }
            else if (file == 5)
            { Assert.AreEqual(P.King, piece.Sym()); }
            else
            { Assert.False(true); }
          }
        }
      }
    }

    [Test]
    public void Places()
    {
      Place place = new Place(4, 4);
      Assert.AreEqual(new Place(4, 5), place.N());
      Assert.AreEqual(new Place(5, 5), place.NE());
      Assert.AreEqual(new Place(5, 4), place.E());
      Assert.AreEqual(new Place(5, 3), place.SE());
      Assert.AreEqual(new Place(4, 3), place.S());
      Assert.AreEqual(new Place(3, 3), place.SW());
      Assert.AreEqual(new Place(3, 4), place.W());
      Assert.AreEqual(new Place(3, 5), place.NW());
    }

    [Test]
    public void BoardCustom()
    {
      Game game = new Game("bnd5");
      Place place = new Place(4, 5);
      Piece knight = game.At(place);
      List<Piece> hand;
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 1; rank <= 8; rank++)
        {
          if (file == 4 && rank == 5)
          {
            Assert.AreEqual(P.Knight, knight.Sym());
            Assert.AreEqual(C.Black, knight.Color());
            Assert.AreEqual(new Place(4, 5), knight.Place());
            continue;
          }
          Assert.AreEqual(null, game.At(new Place(file, rank)));
        }
      }
      hand = game.Hand(C.Black);
      Assert.AreEqual(1, hand.Count);
      Assert.AreEqual(P.Knight, hand[0].Sym());
      Assert.AreEqual(C.Black, hand[0].Color());
      Assert.AreEqual(new Place(4, 5), hand[0].Place());

      // clear one
      game.Clear(place);
      Assert.AreEqual(null, game.At(place));
      Assert.AreEqual(0, hand.Count);

      // put one
      game.Put("bnd5");
      for (int file = 1; file <= 8; file++)
      {
        for (int rank = 1; rank <= 8; rank++)
        {
          if (file == 4 && rank == 5)
          {
            Assert.AreEqual(P.Knight, knight.Sym());
            Assert.AreEqual(C.Black, knight.Color());
            Assert.AreEqual(new Place(4, 5), knight.Place());
            continue;
          }
          Assert.AreEqual(null, game.At(new Place(file, rank)));
        }
      }
      Assert.AreEqual(1, hand.Count);
      Assert.AreEqual(P.Knight, hand[0].Sym());
      Assert.AreEqual(C.Black, hand[0].Color());
      Assert.AreEqual(new Place(4, 5), hand[0].Place());

      // clear all
      game.ClearAll();
      Assert.AreEqual(null, game.At(place));
      Assert.AreEqual(0, hand.Count);
    }
  }

  [TestFixture]
  public class RunControllerTests
  {
    [Test]
    public void Move1WhitePawnRegular()
    {
      Game game = new Game();
      Move move = new Move(game);
      Place src = new Place(2, 2);
      Piece pawn = game.At(src);
      MoveRet ret = move.It(src, src.N());
      Assert.AreEqual(MoveRet.Regular, ret);
      Assert.AreEqual(null, game.At(src));
      Assert.AreEqual(pawn, game.At(src.N()));
    }

    [Test]
    public void Move1BlackPawnBad()
    {
      Game game = new Game();
      Move move = new Move(game);
      Place src = new Place(7, 7);
      MoveRet ret = move.It(src, src.S());
      Assert.AreEqual(MoveRet.BadTurn, ret);
    }

    [Test]
    public void WhitePawnValids()
    {
      Game game = new Game("wpc5");
      Move move = new Move(game);
      Place place = new Place(3, 5);
      Piece pawn = game.At(place);
      List<Place> valids = move.ValidsPawn(pawn);
      Assert.AreEqual(2, valids.Count);
      Assert.AreEqual(place.N(), valids[0]);
      Assert.AreEqual(place.N().N(), valids[1]);

      game.Put("bpc7");
      valids = move.ValidsPawn(pawn);
      Assert.AreEqual(1, valids.Count);
      Assert.AreEqual(place.N(), valids[0]);

      move.It(place, place.N());
      valids = move.ValidsPawn(pawn);
      Assert.AreEqual(0, valids.Count);
    }

    [Test]
    public void WhiteRookValids()
    {
      Game game = new Game("wre4");
      Move move = new Move(game);
      Place place = new Place(5, 4);
      Piece rook = game.At(place);
      List<Place> valids = move.ValidsRook(rook);
      Assert.AreEqual(14, valids.Count);
      // north
      Assert.True(valids.Contains(place.N()));
      Assert.True(valids.Contains(place.N().N()));
      Assert.True(valids.Contains(place.N().N().N()));
      Assert.True(valids.Contains(place.N().N().N().N()));
      // east
      Assert.True(valids.Contains(place.E()));
      Assert.True(valids.Contains(place.E().E()));
      Assert.True(valids.Contains(place.E().E().E()));
      // south
      Assert.True(valids.Contains(place.S()));
      Assert.True(valids.Contains(place.S().S()));
      Assert.True(valids.Contains(place.S().S().S()));
      // west
      Assert.True(valids.Contains(place.W()));
      Assert.True(valids.Contains(place.W().W()));
      Assert.True(valids.Contains(place.W().W().W()));
      Assert.True(valids.Contains(place.W().W().W().W()));

      // north
      game.Put("bpe6");
      valids = move.ValidsRook(rook);
      Assert.AreEqual(12, valids.Count);
      Assert.False(valids.Contains(place.N().N().N()));
      Assert.False(valids.Contains(place.N().N().N().N()));

      // east
      game.Put("wpg4");
      valids = move.ValidsRook(rook);
      Assert.AreEqual(10, valids.Count);
      Assert.False(valids.Contains(place.E().E()));
      Assert.False(valids.Contains(place.E().E().E()));

      // south
      game.Put("wpe2");
      valids = move.ValidsRook(rook);
      Assert.AreEqual(8, valids.Count);
      Assert.False(valids.Contains(place.S().S()));
      Assert.False(valids.Contains(place.S().S().S()));

      // west
      game.Put("bpd4");
      valids = move.ValidsRook(rook);
      Assert.AreEqual(5, valids.Count);
      Assert.False(valids.Contains(place.W().W()));
      Assert.False(valids.Contains(place.W().W().W()));
      Assert.False(valids.Contains(place.W().W().W().W()));
    }

    [Test]
    public void WhiteKnightValids()
    {
      Game game = new Game("wnd5");
      Move move = new Move(game);
      Place place = new Place(4, 5);
      Piece knight = game.At(place);
      List<Place> valids = move.ValidsKnight(knight);
      Assert.AreEqual(8, valids.Count);
      Assert.True(valids.Contains(place.N().NE()));
      Assert.True(valids.Contains(place.E().NE()));
      Assert.True(valids.Contains(place.E().SE()));
      Assert.True(valids.Contains(place.S().SE()));
      Assert.True(valids.Contains(place.S().SW()));
      Assert.True(valids.Contains(place.W().SW()));
      Assert.True(valids.Contains(place.W().NW()));
      Assert.True(valids.Contains(place.N().NW()));

      game.Put("bpc5 bpc4 bpd4");
      valids = move.ValidsKnight(knight);
      Assert.AreEqual(8, valids.Count);

      game.Put("bpe7 wpf6");
      valids = move.ValidsKnight(knight);
      Assert.AreEqual(7, valids.Count);
      Assert.False(valids.Contains(place.E().NE()));

      game.ClearAll();
      game.Put("wna1");
      place = new Place(1, 1);
      knight = game.At(place);
      valids = move.ValidsKnight(knight);
      Assert.AreEqual(2, valids.Count);
      Assert.True(valids.Contains(place.N().NE()));
      Assert.True(valids.Contains(place.E().NE()));
    }

    [Test]
    public void WhiteBishopValids()
    {
      Game game = new Game("wbe4");
      Move move = new Move(game);
      Place place = new Place(5, 4);
      Piece bishop = game.At(place);
      List<Place> valids = move.ValidsBishop(bishop);
      Assert.AreEqual(13, valids.Count);
      // northeast
      Assert.True(valids.Contains(place.NE()));
      Assert.True(valids.Contains(place.NE().NE()));
      Assert.True(valids.Contains(place.NE().NE().NE()));
      // southeast
      Assert.True(valids.Contains(place.SE()));
      Assert.True(valids.Contains(place.SE().SE()));
      Assert.True(valids.Contains(place.SE().SE().SE()));
      // southwest
      Assert.True(valids.Contains(place.SW()));
      Assert.True(valids.Contains(place.SW().SW()));
      Assert.True(valids.Contains(place.SW().SW().SW()));
      // northwest
      Assert.True(valids.Contains(place.NW()));
      Assert.True(valids.Contains(place.NW().NW()));
      Assert.True(valids.Contains(place.NW().NW().NW()));
      Assert.True(valids.Contains(place.NW().NW().NW().NW()));

      // northeast
      game.Put("bpg6");
      valids = move.ValidsBishop(bishop);
      Assert.AreEqual(12, valids.Count);
      Assert.False(valids.Contains(place.NE().NE().NE()));

      // southeast
      game.Put("wpf3");
      valids = move.ValidsBishop(bishop);
      Assert.AreEqual(9, valids.Count);
      Assert.False(valids.Contains(place.SE()));
      Assert.False(valids.Contains(place.SE().SE()));
      Assert.False(valids.Contains(place.SE().SE().SE()));

      // southwest
      game.Put("wpc2");
      valids = move.ValidsBishop(bishop);
      Assert.AreEqual(7, valids.Count);
      Assert.False(valids.Contains(place.SW().SW()));
      Assert.False(valids.Contains(place.SW().SW().SW()));

      // northwest
      game.Put("bpd5");
      valids = move.ValidsBishop(bishop);
      Assert.AreEqual(4, valids.Count);
      Assert.False(valids.Contains(place.NW().NW()));
      Assert.False(valids.Contains(place.NW().NW().NW()));
      Assert.False(valids.Contains(place.NW().NW().NW().NW()));
    }

    [Test]
    public void WhiteQueenValids()
    {
      Game game = new Game("wqd2");
      Move move = new Move(game);
      Place place = new Place(4, 2);
      Piece queen = game.At(place);
      List<Place> valids = move.ValidsQueen(queen);
      Assert.AreEqual(23, valids.Count);
      // north
      Assert.True(valids.Contains(place.N()));
      Assert.True(valids.Contains(place.N().N()));
      Assert.True(valids.Contains(place.N().N().N()));
      Assert.True(valids.Contains(place.N().N().N().N()));
      Assert.True(valids.Contains(place.N().N().N().N().N()));
      Assert.True(valids.Contains(place.N().N().N().N().N().N()));
      // northeast
      Assert.True(valids.Contains(place.NE()));
      Assert.True(valids.Contains(place.NE().NE()));
      Assert.True(valids.Contains(place.NE().NE().NE()));
      Assert.True(valids.Contains(place.NE().NE().NE().NE()));
      // east
      Assert.True(valids.Contains(place.E()));
      Assert.True(valids.Contains(place.E().E()));
      Assert.True(valids.Contains(place.E().E().E()));
      Assert.True(valids.Contains(place.E().E().E().E()));
      // southeast
      Assert.True(valids.Contains(place.SE()));
      // south
      Assert.True(valids.Contains(place.S()));
      // southwest
      Assert.True(valids.Contains(place.SW()));
      // west
      Assert.True(valids.Contains(place.W()));
      Assert.True(valids.Contains(place.W().W()));
      Assert.True(valids.Contains(place.W().W().W()));
      // northwest
      Assert.True(valids.Contains(place.NW()));
      Assert.True(valids.Contains(place.NW().NW()));
      Assert.True(valids.Contains(place.NW().NW().NW()));
    }

    [Test]
    public void WhiteKingValids()
    {
      Game game = new Game("wkd3");
      Move move = new Move(game);
      Place place = new Place(4, 3);
      Piece king = game.At(place);
      List<Place> valids = move.ValidsKing(king);
      Assert.AreEqual(8, valids.Count);
      Assert.True(valids.Contains(place.N()));
      Assert.True(valids.Contains(place.NE()));
      Assert.True(valids.Contains(place.E()));
      Assert.True(valids.Contains(place.SE()));
      Assert.True(valids.Contains(place.S()));
      Assert.True(valids.Contains(place.SW()));
      Assert.True(valids.Contains(place.W()));
      Assert.True(valids.Contains(place.NW()));

      game.Put("bbb6");
      valids = move.ValidsKing(king);
      Assert.AreEqual(6, valids.Count);
      Assert.False(valids.Contains(place.N()));
      Assert.False(valids.Contains(place.E()));
    }
  }

  [TestFixture]
  public class RunUtilTests
  {
    [Test]
    public void ParsePieces()
    {
      List<Piece> pieces;
      int ret = Helper.ParsePieces("wqf8 bpc4", out pieces);
      Assert.AreEqual(0, ret);
      Assert.AreEqual(2, pieces.Count);
      Assert.AreEqual(P.Queen, pieces[0].Sym());
      Assert.AreEqual(C.White, pieces[0].Color());
      Assert.AreEqual(new Place(6, 8), pieces[0].Place());
      Assert.AreEqual(P.Pawn, pieces[1].Sym());
      Assert.AreEqual(C.Black, pieces[1].Color());
      Assert.AreEqual(new Place(3, 4), pieces[1].Place());
    }
  }
}
