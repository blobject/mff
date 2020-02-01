using System;
using System.Collections.Generic;
using Chesss.Model;

namespace Chesss.Controller
{
  public enum MoveRet
  {
    BadSrc,
    BadDst,
    BadTurn,
    BadMove,
    Regular,
    Capture
  }

  public enum WhoRet
  {
    Noone,
    Enemy,
    Friend,
    Oob, // "out of bounds"
    Error
  }

  public class Move
  {
    private Game game;

    public Move(Game game)
    {
      this.game = game;
    }

    public MoveRet
    It(Place src, Place dst)
    {
      Piece srcPiece = this.game.At(src);
      Piece dstPiece = this.game.At(dst);

      if (srcPiece == null)
      {
        return MoveRet.BadSrc;
      }

      if (srcPiece.Color() != this.game.Turn())
      {
        return MoveRet.BadTurn;
      }

      if (dstPiece != null && dstPiece.Color() == this.game.Turn())
      {
        return MoveRet.BadDst;
      }

      if (!Valid(srcPiece, dst))
      {
        return MoveRet.BadMove;
      }

      MoveRet ret;
      if (dstPiece != null)
      {
        List<Piece> hand = game.Hand(game.Other());
        List<Piece> captives = game.Captives(game.Turn());
        hand.Remove(dstPiece);
        captives.Add(dstPiece);
        ret = MoveRet.Capture;
      }
      else
      {
        ret = MoveRet.Regular;
      }

      this.game.Set(src, null);
      this.game.Set(dst, srcPiece);
      srcPiece.Quicken();
      this.game.Toggle();
      return ret;
    }

    public WhoRet
    Who(Piece that)
    {
      if (that == null)
      {
        return WhoRet.Noone;
      }
      if (this.game.Turn() != that.Color())
      {
        return WhoRet.Enemy;
      }
      if (this.game.Turn() == that.Color())
      {
        return WhoRet.Friend;
      }
      return WhoRet.Error;
    }

    public WhoRet
    Step(string dir, Place here, out Place there)
    {
      there = null;
      int file = here.File();
      int rank = here.Rank();
      if      (dir == "n")  { rank++; }
      else if (dir == "ne") { file++; rank++; }
      else if (dir == "e")  { file++; }
      else if (dir == "se") { file++; rank--; }
      else if (dir == "s")  { rank--; }
      else if (dir == "sw") { file--; rank--; }
      else if (dir == "w")  { file--; }
      else if (dir == "nw") { file--; rank++; }
      if (file < 1 || file > 8 || rank < 1 || rank > 8)
      {
        return WhoRet.Oob;
      }
      there = new Place(file, rank);
      return Who(this.game.At(new Place(file, rank)));
    }

    public List<Place>
    ValidsPawn(Piece pawn)
    {
      Place place = pawn.Place();
      int file = place.File();
      int rank = place.Rank();
      string dir = "n";
      if (pawn.Color() == C.Black)
      {
        dir = "s";
      }
      var valids = new List<Place>();

      if (Step(dir, pawn.Place(), out place) == WhoRet.Noone)
      {
        valids.Add(place);
      }
      if (pawn.Inert() && Step(dir, place, out place) == WhoRet.Noone)
      {
        place = new Place(file, rank + 2);
        if (pawn.Color() == C.Black)
        {
          place = new Place(file, rank - 2);
        }
        valids.Add(place);
      }
      return valids;
    }

    public List<Place>
    ValidsRook(Piece rook)
    {
      var valids = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in new string[] {"n", "e", "s", "w"})
      {
        place = rook.Place();
        while ((ret = Step(dir, place, out place)) == WhoRet.Noone)
        {
          valids.Add(place);
        }
        if (ret == WhoRet.Enemy)
        {
          valids.Add(place);
        }
      }
      return valids;
    }

    public List<Place>
    ValidsKnight(Piece knight)
    {
      var valids = new List<Place>();
      WhoRet ret;
      Place place;
      Place trace;
      string left;
      string right;
      foreach (var dir in new string[] {"n", "e", "s", "w"})
      {
        place = knight.Place();
        ret = Step(dir, place, out place);
        if (ret == WhoRet.Oob)
        {
          continue;
        }
        ret = Step(dir, place, out trace);
        if (ret == WhoRet.Oob)
        {
          continue;
        }
        if (dir == "n" || dir == "s")
        {
          left = "w";
          right = "e";
        }
        else
        {
          left = "n";
          right = "s";
        }
        foreach (var dir2 in new string[] {left, right})
        {
          ret = Step(dir2, trace, out place);
          if (ret == WhoRet.Noone || ret == WhoRet.Enemy)
          {
            valids.Add(place);
          }
        }
      }
      return valids;
    }

    public List<Place>
    ValidsBishop(Piece bishop)
    {
      var valids = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in new string[] {"ne", "se", "sw", "nw"})
      {
        place = bishop.Place();
        while ((ret = Step(dir, place, out place)) == WhoRet.Noone)
        {
          valids.Add(place);
        }
        if (ret == WhoRet.Enemy)
        {
          valids.Add(place);
        }
      }
      return valids;
    }

    public List<Place>
    ValidsQueen(Piece queen)
    {
      var valids = ValidsRook(queen);
      foreach (var valid in ValidsBishop(queen))
      {
        valids.Add(valid);
      }
      return valids;
    }

    public List<Place>
    ValidsKing(Piece king)
    {
      var valids = new List<Place>();
      var threats = new HashSet<Place>();
      var safes = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in
               new string[] {"n", "ne", "e", "se", "s", "sw", "w", "nw"})
      {
        place = king.Place();
        ret = Step(dir, place, out place);
        if (ret == WhoRet.Noone || ret == WhoRet.Enemy)
        {
          valids.Add(place);
        }
      }
      // TODO: capture but threatened

      // consider threats
      foreach (var enemy in game.Hand(game.Other()))
      {
        foreach (var threat in Valids(enemy))
        {
          threats.Add(threat);
        }
      }
      foreach (var valid in valids)
      {
        if (!threats.Contains(valid))
        {
          safes.Add(valid);
        }
      }

      return safes;
    }

    public List<Place>
    Valids(Piece piece)
    {
      List<Place> valids;
      P sym = piece.Sym();
      if (sym == P.Pawn)
      {
        valids = ValidsPawn(piece);
      }
      else if (sym == P.Rook)
      {
        valids = ValidsRook(piece);
      }
      else if (sym == P.Knight)
      {
        valids = ValidsKnight(piece);
      }
      else if (sym == P.Bishop)
      {
        valids = ValidsBishop(piece);
      }
      else if (sym == P.Queen)
      {
        valids = ValidsQueen(piece);
      }
      else
      {
        valids = ValidsKing(piece);
      }
      return valids;
    }

    public bool
    Valid(Piece piece, Place dst)
    {
      List<Place> valids = Valids(piece);
      return valids.Contains(dst);
    }
  }
}
