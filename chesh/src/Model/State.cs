using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json;
using Chesh.Util;
using Chesh.View;

namespace Chesh.Model
{
  public class State
  {
    public char[,] Board;
    public Piece Selection { get; set; }
    public List<Piece> Live { get; set; }
    public List<Piece> Dead { get; set; }
    public List<(string,long)> History { get; set; }
    public State()
    {
      var live = new List<Piece>();
      live.Add(new Rook(Color.White, 1, 1));
      live.Add(new Knight(Color.White, 2, 1));
      live.Add(new Bishop(Color.White, 3, 1));
      live.Add(new Queen(Color.White, 4, 1));
      live.Add(new King(Color.White, 5, 1));
      live.Add(new Bishop(Color.White, 6, 1));
      live.Add(new Knight(Color.White, 7, 1));
      live.Add(new Rook(Color.White, 8, 1));
      for (int file = 1; file <= 8; file++)
      {
        live.Add(new Pawn(Color.White, file, 2));
      }
      for (int file = 1; file <= 8; file++)
      {
        live.Add(new Pawn(Color.Black, file, 7));
      }
      live.Add(new Rook(Color.Black, 1, 8));
      live.Add(new Knight(Color.Black, 2, 8));
      live.Add(new Bishop(Color.Black, 3, 8));
      live.Add(new Queen(Color.Black, 4, 8));
      live.Add(new King(Color.Black, 5, 8));
      live.Add(new Bishop(Color.Black, 6, 8));
      live.Add(new Knight(Color.Black, 7, 8));
      live.Add(new Rook(Color.Black, 8, 8));
      this.Live = live;
      this.Dead = new List<Piece>();
      this.History = new List<(string,long)>();
      this.UpdateBoard(out this.Board);
    }

    // accessors ///////////////////////////////////////////////////////////////

    public List<Piece>
    AllPieces()
    {
      var pieces = new List<Piece>();
      pieces.AddRange(this.Live);
      pieces.AddRange(this.Dead);
      return pieces;
    }

    public Piece
    At(int x, int y)
    {
      foreach (var piece in this.Live)
      {
        if (piece.X == x && piece.Y == y)
        {
          return piece;
        }
      }
      return null;
    }

    public Color
    Turn(bool current)
    {
      if (this.History.Count % 2 == 0)
      {
        if (current)
        {
          return Color.White;
        }
        return Color.Black;
      }
      if (current)
      {
        return Color.Black;
      }
      return Color.White;
    }

    // mutators ////////////////////////////////////////////////////////////////

    public void
    UpdateBoard(out char[,] board)
    {
      // this should not be a user-mutation;
      // should just get called after every move.
      board = new char[8, 8];
      foreach (var piece in this.Live)
      {
        board[piece.X - 1, piece.Y - 1] = piece.Sym[0];
      }
    }

    public void
    Kill(int x, int y)
    {
      foreach (var piece in this.Live)
      {
        if (piece.X == x && piece.Y == y)
        {
          this.Live.Remove(piece);
          return;
        }
      }
    }

    // player actions //////////////////////////////////////////////////////////

    public Ret
    Select(int x, int y)
    {
      Piece piece = this.At(x, y);
      if (piece == null)
      {
        return Ret.BadSrc;
      }
      if (piece.Color != this.Turn(true))
      {
        return Ret.BadTurn;
      }
      this.Selection = piece;
      return Ret.Selected;
    }

    public string
    LastNote()
    {
      if (this.History.Count > 0)
      {
        return this.History[this.History.Count - 1].Item1;
      }
      return null;
    }

    public List<(int,int,int,int)>
    LineOfAttack(Piece attacker, Piece target)
    {
      List<(int,int,int,int)> threat = null;
      var targetSwap = (target.X, target.Y, target.X, target.Y);
      string last = null;
      if (attacker.GetType().Name == "Pawn")
      {
        last = this.LastNote();
      }
      foreach (var ray in attacker.Rays(this.Board, last))
      {
        if (ray.Contains(targetSwap))
        {
          threat = ray;
          break;
        }
      }
      threat.Remove(targetSwap);
      threat.Add((attacker.X, attacker.Y, attacker.X, attacker.Y));
      return threat;
    }

    public bool
    WillCheck(Color turn, Piece mover, int x, int y)
    {
      bool check = false;

      // tentative move
      int xOld = mover.X;
      int yOld = mover.Y;
      mover.X = x;
      mover.Y = y;
      this.UpdateBoard(out this.Board);
      this.Board[x - 1, y - 1] = mover.Sym[0]; // in case of more than one

      // will it threaten the enemy king?
      Piece enemyKing = null;
      foreach (var piece in this.Live)
      {
        if (piece.GetType().Name == "King" && piece.Color != turn)
        {
          enemyKing = piece;
          break;
        }
      }

      foreach (var swap in mover.Reach(this.Board, this.LastNote()))
      {
        if (swap.Item3 == enemyKing.X && swap.Item4 == enemyKing.Y)
        {
          check = true;
        }
      }

      // backtrack
      mover.X = xOld;
      mover.Y = yOld;
      this.UpdateBoard(out this.Board);

      return check;
    }

    public bool
    WillBeChecked(Color turn, Piece mover, int x, int y)
    {
      bool check = false;
      string last = this.LastNote();

      // tentative move
      int xOld = mover.X;
      int yOld = mover.Y;
      mover.X = x;
      mover.Y = y;
      this.UpdateBoard(out this.Board);
      this.Board[x - 1, y - 1] = mover.Sym[0]; // in case of more than one

      // will the king be threatened?
      Piece king = null;
      foreach (var piece in this.Live)
      {
        if (piece.GetType().Name == "King" && piece.Color == turn)
        {
          king = piece;
          break;
        }
      }
      foreach (var piece in this.Live)
      {
        if (piece.Color == turn)
        {
          continue;
        }
        if (piece.X == x && piece.Y == y)
        {
          continue;
        }
        foreach (var swap in piece.Reach(this.Board, last))
        {
          if (swap.Item1 == king.X && swap.Item2 == king.Y)
          {
            check = true;
            break;
          }
        }
        if (check)
        {
          break;
        }
      }

      // backtrack
      mover.X = xOld;
      mover.Y = yOld;
      this.UpdateBoard(out this.Board);

      return check;
    }

    public bool
    WillCheckmate(Color turn, Piece mover, int x, int y)
    {
      bool checkmate = true;
      string last = this.LastNote();

      // tentative move
      int xOld = mover.X;
      int yOld = mover.Y;
      mover.X = x;
      mover.Y = y;
      this.UpdateBoard(out this.Board);
      this.Board[x - 1, y - 1] = mover.Sym[0]; // in case of more than one

      // can the enemy king move somewhere safe?
      Piece enemyKing = null;
      foreach (var piece in this.Live)
      {
        if (piece.GetType().Name == "King" && piece.Color != turn)
        {
          enemyKing = piece;
          break;
        }
      }
      foreach (var swap in enemyKing.Reach(this.Board, null))
      {
        if (! this.WillBeChecked(this.Turn(false), enemyKing,
                                 swap.Item1, swap.Item2))
        {
          checkmate = false;
          break;
        }
      }

      // get line of attack - we do this here, before backtracking
      List<(int,int,int,int)> threat = null;
      if (checkmate)
      {
        threat = this.LineOfAttack(mover, enemyKing);
      }

      // backtrack
      mover.X = xOld;
      mover.Y = yOld;
      this.UpdateBoard(out this.Board);

      if (! checkmate)
      {
        return false;
      }

      // tentative move
      mover.X = x;
      mover.Y = y;
      this.UpdateBoard(out this.Board);
      this.Board[x - 1, y - 1] = mover.Sym[0]; // in case of more than one

      // can any friendly block the threat?
      foreach (var swap in threat)
      {
        foreach (var piece in this.Live)
        {
          if (piece.Color == turn)
          {
            continue;
          }
          if (piece.X == x && piece.Y == y)
          {
            continue;
          }
          foreach (var reach in piece.Reach(this.Board, last))
          {
            if (x == 4 && y == 7)
            {
            if (reach == swap) Console.Write("TRUE ");
            }
            if (reach == swap &&
                ! this.WillBeChecked(this.Turn(false), piece,
                                     reach.Item1, reach.Item2))
            {
              checkmate = false;
              break;
            }
          }
        }
        if (! checkmate)
        {
          break;
        }
      }

      // backtrack
      mover.X = xOld;
      mover.Y = yOld;
      this.UpdateBoard(out this.Board);

      return checkmate;
    }

    public void
    Promote(List<Ret> rets, string prom, int xSrc, int ySrc, int xDst, int yDst)
    {
      Color turn = this.Turn(true);
      Piece piece;
      if (prom == "R")
      {
        piece = new Rook(turn, xDst, yDst);
      }
      else if (prom == "N")
      {
        piece = new Knight(turn, xDst, yDst);
      }
      else if (prom == "B")
      {
        piece = new Bishop(turn, xDst, yDst);
      }
      else // "Q"
      {
        piece = new Queen(turn, xDst, yDst);
      }
      this.Kill(xSrc, ySrc);
      this.Live.Add(piece);
      if (this.WillCheck(turn, piece, xDst, yDst))
      {
        rets.Add(Ret.Check);
        if (this.WillCheckmate(turn, piece, xDst, yDst))
        {
          rets.Remove(Ret.Check);
          rets.Add(Ret.Checkmate);
        }
      }
      this.Upstate(rets, piece, "P", prom, xSrc, ySrc, xDst, yDst, 0, 0);
    }

    public List<Ret>
    Move(int xSrc, int ySrc, int xDst, int yDst)
    {
      Piece src = this.At(xSrc, ySrc);
      Piece dst = this.At(xDst, yDst);

      // get simple bad moves out of the way
      if (src == null)
      {
        return new List<Ret>() { Ret.BadSrc };
      }
      if (src.Color != this.Turn(true))
      {
        return new List<Ret>() { Ret.BadTurn };
      }
      if (dst != null && dst.Color == this.Turn(true))
      {
        return new List<Ret>() { Ret.BadDst };
      }

      // try making the move
      // note: this is the first creation of <Ret>s
      int xMore;
      int yMore;
      var rets = src.Move(this, xDst, yDst, out xMore, out yMore);

      // bad moves that are a bit more involved
      // note: original <Ret>s can be ignored
      if (rets.Contains(Ret.InvalidMove))
      {
        return rets;
      }
      if (this.WillBeChecked(this.Turn(true), src, xDst, yDst))
      {
        return new List<Ret>() { Ret.Checked };
      }

      // good moves
      // note: must add to the first <Ret>s
      string prom = "";
      if (src.GetType().Name == "Pawn" &&
          ((src.Color == Color.Black && yDst == 1) ||
           (src.Color == Color.White && yDst == 8)))
      {
        rets.Add(Ret.Promote);
        return rets;
      }
      if (this.WillCheck(this.Turn(true), src, xDst, yDst))
      {
        rets.Add(Ret.Check);
        if (this.WillCheckmate(this.Turn(true), src, xDst, yDst))
        {
          rets.Remove(Ret.Check);
          rets.Add(Ret.Checkmate);
        }
      }

      // mutate state
      this.Upstate(rets, src, src.Sym, prom, xSrc, ySrc,
                   xDst, yDst, xMore, yMore);
      return rets;
    }

    public void
    Upstate(List<Ret> rets,
            Piece src, string sym, string prom,
            int xSrc, int ySrc,
            int xDst, int yDst,
            int xMore, int yMore)
    {
      src.Inert = false;
      this.Selection = null;
      this.History.Add((Helper.Notate(rets, sym, prom,
                                      xSrc, ySrc, xDst, yDst),
                        (new DateTime(DateTime.Now.Ticks)).Ticks));
      if (xMore > 0 && yMore > 0)
      {
        if (rets.Contains(Ret.Capture) || rets.Contains(Ret.EnPassant))
        {
          this.Dead.Add(this.At(xMore, yMore));
          this.Kill(xMore, yMore);
        }
        if (rets.Contains(Ret.Castle))
        {
          Piece rook = this.At(xMore, yMore);
          rook.X = xDst - 1; // kingside
          if (xDst < 5)
          {
            rook.X = xDst + 1; // queenside
          }
        }
      }
      src.X = xDst;
      src.Y = yDst;
      this.UpdateBoard(out this.Board);
    }
  }
}
