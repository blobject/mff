// EXECUTION (on linux):
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
  public class RunBoardTests
  {
    [Test]
    public void BoardInit()
    {
      Game game = new Game();
      Place place;
      Piece piece;

      Assert.AreEqual(C.White, game.Turn);
      Assert.AreEqual(0, game.Log.Count);
      Assert.AreEqual(8, Board.Width);
      Assert.AreEqual(8, Board.Height);

      for (int file = 1; file <= Board.Width; file++)
      {
        for (int rank = 1; rank <= Board.Height; rank++)
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
            Assert.AreEqual(C.White, piece.Color);
          }
          // black pieces
          else if (rank >= 7)
          {
            Assert.AreEqual(C.Black, piece.Color);
          }
          // commonalities
          Assert.True(piece.Inert);
          Assert.AreEqual(place, piece.Place);
          if (rank == 2 || rank == 7)
          {
            Assert.IsInstanceOf<Pawn>(piece);
          }
          else if (rank == 1 || rank == 8)
          {
            if (file == 1 || file == 8)
            { Assert.IsInstanceOf<Rook>(piece); }
            else if (file == 2 || file == 7)
            { Assert.IsInstanceOf<Knight>(piece); }
            else if (file == 3 || file == 6)
            { Assert.IsInstanceOf<Bishop>(piece); }
            else if (file == 4)
            { Assert.IsInstanceOf<Queen>(piece); }
            else if (file == 5)
            { Assert.IsInstanceOf<King>(piece); }
            else
            { Assert.False(true); }
          }
        }
      }
    }

    [Test]
    public void BoardCustom()
    {
      Game game = new Game("bnd5");
      Place place = new Place(4, 5);
      Knight knight = (Knight) game.At(place);
      List<Piece> hand;
      for (int file = 1; file <= Board.Width; file++)
      {
        for (int rank = 1; rank <= Board.Height; rank++)
        {
          if (file == 4 && rank == 5)
          {
            Assert.IsInstanceOf<Knight>(knight);
            Assert.AreEqual(C.Black, knight.Color);
            Assert.AreEqual(new Place(4, 5), knight.Place);
            continue;
          }
          Assert.AreEqual(null, game.At(new Place(file, rank)));
        }
      }
      hand = game.Hand(C.Black);
      Assert.AreEqual(1, hand.Count);
      Assert.IsInstanceOf<Knight>(hand[0]);
      Assert.AreEqual(C.Black, hand[0].Color);
      Assert.AreEqual(new Place(4, 5), hand[0].Place);

      // clear one
      game.Clear(place);
      Assert.AreEqual(null, game.At(place));
      Assert.AreEqual(0, hand.Count);

      // put one
      game.Put("bnd5");
      for (int file = 1; file <= Board.Width; file++)
      {
        for (int rank = 1; rank <= Board.Height; rank++)
        {
          if (file == 4 && rank == 5)
          {
            Assert.IsInstanceOf<Knight>(knight);
            Assert.AreEqual(C.Black, knight.Color);
            Assert.AreEqual(new Place(4, 5), knight.Place);
            continue;
          }
          Assert.AreEqual(null, game.At(new Place(file, rank)));
        }
      }
      Assert.AreEqual(1, hand.Count);
      Assert.IsInstanceOf<Knight>(hand[0]);
      Assert.AreEqual(C.Black, hand[0].Color);
      Assert.AreEqual(new Place(4, 5), hand[0].Place);

      // clear all
      game.ClearAll();
      Assert.AreEqual(null, game.At(place));
      Assert.AreEqual(0, hand.Count);
    }
  }

  [TestFixture]
  public class RunPieceTests
  {
    [Test]
    public void WhitePawnReach()
    {
      Game game = new Game("wpc5");
      Move move = new Move(game);
      Place place = new Place(3, 5);
      Pawn pawn = (Pawn) game.At(place);
      List<Place> reach = pawn.Reach(game);
      Assert.AreEqual(2, reach.Count);
      Assert.AreEqual(place.N(), reach[0]);
      Assert.AreEqual(place.N().N(), reach[1]);

      game.Put("bpc7");
      reach = pawn.Reach(game);
      Assert.AreEqual(1, reach.Count);
      Assert.AreEqual(place.N(), reach[0]);

      move.It(place, place.N());
      reach = pawn.Reach(game);
      Assert.AreEqual(0, reach.Count);
    }

    [Test]
    public void WhiteRookReach()
    {
      Game game = new Game("wre4");
      Place place = new Place(5, 4);
      Rook rook = (Rook) game.At(place);
      List<Place> reach = rook.Reach(game);
      Assert.AreEqual(14, reach.Count);
      // north
      Assert.True(reach.Contains(place.N()));
      Assert.True(reach.Contains(place.N().N()));
      Assert.True(reach.Contains(place.N().N().N()));
      Assert.True(reach.Contains(place.N().N().N().N()));
      // east
      Assert.True(reach.Contains(place.E()));
      Assert.True(reach.Contains(place.E().E()));
      Assert.True(reach.Contains(place.E().E().E()));
      // south
      Assert.True(reach.Contains(place.S()));
      Assert.True(reach.Contains(place.S().S()));
      Assert.True(reach.Contains(place.S().S().S()));
      // west
      Assert.True(reach.Contains(place.W()));
      Assert.True(reach.Contains(place.W().W()));
      Assert.True(reach.Contains(place.W().W().W()));
      Assert.True(reach.Contains(place.W().W().W().W()));

      // north
      game.Put("bpe6");
      reach = rook.Reach(game);
      Assert.AreEqual(12, reach.Count);
      Assert.False(reach.Contains(place.N().N().N()));
      Assert.False(reach.Contains(place.N().N().N().N()));

      // east
      game.Put("wpg4");
      reach = rook.Reach(game);
      Assert.AreEqual(10, reach.Count);
      Assert.False(reach.Contains(place.E().E()));
      Assert.False(reach.Contains(place.E().E().E()));

      // south
      game.Put("wpe2");
      reach = rook.Reach(game);
      Assert.AreEqual(8, reach.Count);
      Assert.False(reach.Contains(place.S().S()));
      Assert.False(reach.Contains(place.S().S().S()));

      // west
      game.Put("bpd4");
      reach = rook.Reach(game);
      Assert.AreEqual(5, reach.Count);
      Assert.False(reach.Contains(place.W().W()));
      Assert.False(reach.Contains(place.W().W().W()));
      Assert.False(reach.Contains(place.W().W().W().W()));
    }

    [Test]
    public void WhiteKnightReach()
    {
      Game game = new Game("wnd5");
      Place place = new Place(4, 5);
      Knight knight = (Knight) game.At(place);
      List<Place> reach = knight.Reach(game);
      Assert.AreEqual(8, reach.Count);
      Assert.True(reach.Contains(place.N().NE()));
      Assert.True(reach.Contains(place.E().NE()));
      Assert.True(reach.Contains(place.E().SE()));
      Assert.True(reach.Contains(place.S().SE()));
      Assert.True(reach.Contains(place.S().SW()));
      Assert.True(reach.Contains(place.W().SW()));
      Assert.True(reach.Contains(place.W().NW()));
      Assert.True(reach.Contains(place.N().NW()));

      game.Put("bpc5 bpc4 bpd4");
      reach = knight.Reach(game);
      Assert.AreEqual(8, reach.Count);

      game.Put("bpe7 wpf6");
      reach = knight.Reach(game);
      Assert.AreEqual(7, reach.Count);
      Assert.False(reach.Contains(place.E().NE()));

      game.ClearAll();
      game.Put("wna1");
      place = new Place(1, 1);
      knight = (Knight) game.At(place);
      reach = knight.Reach(game);
      Assert.AreEqual(2, reach.Count);
      Assert.True(reach.Contains(place.N().NE()));
      Assert.True(reach.Contains(place.E().NE()));
    }

    [Test]
    public void WhiteBishopReach()
    {
      Game game = new Game("wbe4");
      Place place = new Place(5, 4);
      Bishop bishop = (Bishop) game.At(place);
      List<Place> reach = bishop.Reach(game);
      Assert.AreEqual(13, reach.Count);
      // northeast
      Assert.True(reach.Contains(place.NE()));
      Assert.True(reach.Contains(place.NE().NE()));
      Assert.True(reach.Contains(place.NE().NE().NE()));
      // southeast
      Assert.True(reach.Contains(place.SE()));
      Assert.True(reach.Contains(place.SE().SE()));
      Assert.True(reach.Contains(place.SE().SE().SE()));
      // southwest
      Assert.True(reach.Contains(place.SW()));
      Assert.True(reach.Contains(place.SW().SW()));
      Assert.True(reach.Contains(place.SW().SW().SW()));
      // northwest
      Assert.True(reach.Contains(place.NW()));
      Assert.True(reach.Contains(place.NW().NW()));
      Assert.True(reach.Contains(place.NW().NW().NW()));
      Assert.True(reach.Contains(place.NW().NW().NW().NW()));

      // northeast
      game.Put("bpg6");
      reach = bishop.Reach(game);
      Assert.AreEqual(12, reach.Count);
      Assert.False(reach.Contains(place.NE().NE().NE()));

      // southeast
      game.Put("wpf3");
      reach = bishop.Reach(game);
      Assert.AreEqual(9, reach.Count);
      Assert.False(reach.Contains(place.SE()));
      Assert.False(reach.Contains(place.SE().SE()));
      Assert.False(reach.Contains(place.SE().SE().SE()));

      // southwest
      game.Put("wpc2");
      reach = bishop.Reach(game);
      Assert.AreEqual(7, reach.Count);
      Assert.False(reach.Contains(place.SW().SW()));
      Assert.False(reach.Contains(place.SW().SW().SW()));

      // northwest
      game.Put("bpd5");
      reach = bishop.Reach(game);
      Assert.AreEqual(4, reach.Count);
      Assert.False(reach.Contains(place.NW().NW()));
      Assert.False(reach.Contains(place.NW().NW().NW()));
      Assert.False(reach.Contains(place.NW().NW().NW().NW()));
    }

    [Test]
    public void WhiteQueenReach()
    {
      Game game = new Game("wqd2");
      Place place = new Place(4, 2);
      Queen queen = (Queen) game.At(place);
      List<Place> reach = queen.Reach(game);
      Assert.AreEqual(23, reach.Count);
      // north
      Assert.True(reach.Contains(place.N()));
      Assert.True(reach.Contains(place.N().N()));
      Assert.True(reach.Contains(place.N().N().N()));
      Assert.True(reach.Contains(place.N().N().N().N()));
      Assert.True(reach.Contains(place.N().N().N().N().N()));
      Assert.True(reach.Contains(place.N().N().N().N().N().N()));
      // northeast
      Assert.True(reach.Contains(place.NE()));
      Assert.True(reach.Contains(place.NE().NE()));
      Assert.True(reach.Contains(place.NE().NE().NE()));
      Assert.True(reach.Contains(place.NE().NE().NE().NE()));
      // east
      Assert.True(reach.Contains(place.E()));
      Assert.True(reach.Contains(place.E().E()));
      Assert.True(reach.Contains(place.E().E().E()));
      Assert.True(reach.Contains(place.E().E().E().E()));
      // southeast
      Assert.True(reach.Contains(place.SE()));
      // south
      Assert.True(reach.Contains(place.S()));
      // southwest
      Assert.True(reach.Contains(place.SW()));
      // west
      Assert.True(reach.Contains(place.W()));
      Assert.True(reach.Contains(place.W().W()));
      Assert.True(reach.Contains(place.W().W().W()));
      // northwest
      Assert.True(reach.Contains(place.NW()));
      Assert.True(reach.Contains(place.NW().NW()));
      Assert.True(reach.Contains(place.NW().NW().NW()));
    }

    [Test]
    public void WhiteKingReach()
    {
      Game game = new Game("wkd3");
      Place place = new Place(4, 3);
      King king = (King) game.At(place);
      List<Place> reach = king.Reach(game);
      Assert.AreEqual(8, reach.Count);
      Assert.True(reach.Contains(place.N()));
      Assert.True(reach.Contains(place.NE()));
      Assert.True(reach.Contains(place.E()));
      Assert.True(reach.Contains(place.SE()));
      Assert.True(reach.Contains(place.S()));
      Assert.True(reach.Contains(place.SW()));
      Assert.True(reach.Contains(place.W()));
      Assert.True(reach.Contains(place.NW()));

      game.Put("bbb6");
      reach = king.Reach(game);
      Assert.AreEqual(6, reach.Count);
      Assert.False(reach.Contains(place.N()));
      Assert.False(reach.Contains(place.E()));
    }
  }

  [TestFixture]
  public class RunPlaceTests
  {
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
      Assert.AreEqual(new Place(4, 6), place.N().N());
    }
  }

  [TestFixture]
  public class RunMoveTests
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
  }

  [TestFixture]
  public class RunUtilTests
  {
    [Test]
    public void FileConversion()
    {
      Assert.AreEqual('a', Helper.ToFileChar(1));
      Assert.AreEqual('g', Helper.ToFileChar(7));
      Assert.AreEqual(2, Helper.ToFileNum('b'));
      Assert.AreEqual(8, Helper.ToFileNum('h'));
    }

    [Test]
    public void ParsePieces()
    {
      List<Piece> pieces;
      int ret = Helper.ParsePieces("xqf8", out pieces);
      Assert.AreEqual(1, ret);
      ret = Helper.ParsePieces("wxf8", out pieces);
      Assert.AreEqual(1, ret);
      ret = Helper.ParsePieces("wqf8 bpc4", out pieces);
      Assert.AreEqual(0, ret);
      Assert.AreEqual(2, pieces.Count);
      Assert.IsInstanceOf<Queen>(pieces[0]);
      Assert.AreEqual(C.White, pieces[0].Color);
      Assert.AreEqual(new Place(6, 8), pieces[0].Place);
      Assert.IsInstanceOf<Pawn>(pieces[1]);
      Assert.AreEqual(C.Black, pieces[1].Color);
      Assert.AreEqual(new Place(3, 4), pieces[1].Place);
    }
  }
}
