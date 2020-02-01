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
    public Game Game { get; set; }

    public Move(Game game)
    {
      this.Game = game;
    }

    public MoveRet
    Try(Place src, Place dst)
    {
      Piece srcPiece = this.Game.At(src);
      Piece dstPiece = this.Game.At(dst);

      if (srcPiece == null)
      {
        return MoveRet.BadSrc;
      }

      if (srcPiece.Color != this.Game.Turn)
      {
        return MoveRet.BadTurn;
      }

      if (dstPiece != null && dstPiece.Color == this.Game.Turn)
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
        return MoveRet.Capture;
      }

      return MoveRet.Regular;
    }

    public MoveRet
    It(Place src, Place dst)
    {
      Piece srcPiece = this.Game.At(src);
      Piece dstPiece = this.Game.At(dst);
      MoveRet ret = MoveRet.Regular;

      if (dstPiece != null)
      {
        List<Piece> hand = this.Game.Hand(this.Game.Other());
        List<Piece> captives = this.Game.Captives(this.Game.Turn);
        hand.Remove(dstPiece);
        captives.Add(dstPiece);
        ret = MoveRet.Capture;
      }

      this.Game.Set(src, null);
      this.Game.Set(dst, srcPiece);
      srcPiece.Quicken();
      srcPiece.Place = dst;
      this.Game.Toggle();
      return ret;
    }

    public bool
    Valid(Piece piece, Place dst)
    {
      return piece.Reach(this.Game).Contains(dst);
    }
  }
}
