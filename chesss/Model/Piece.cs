using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public abstract class Piece
  {
    protected State State;
    public string Sym {get; set; }
    public Color Color {get; set; }
    public int X {get; set; }
    public int Y {get; set; }
    public bool Inert {get; set; }

    public Piece(State state,
                 string sym,
                 Color color,
                 int x, int y,
                 bool inert)
    {
      this.State = state;
      this.Sym = sym;
      this.Color = color;
      this.X = x;
      this.Y = y;
      this.Inert = inert;
    }
  }

  public class Pawn : Piece
  {
    public Pawn(State state, Color color, int x, int y)
      : base(state, "p", color, x, y, true) {}
  }

  public class Rook : Piece
  {
    public Rook(State state, Color color, int x, int y)
      : base(state, "r", color, x, y, true) {}
  }

  public class Knight : Piece
  {
    public Knight(State state, Color color, int x, int y)
      : base(state, "n", color, x, y, true) {}
  }

  public class Bishop : Piece
  {
    public Bishop(State state, Color color, int x, int y)
      : base(state, "b", color, x, y, true) {}
  }

  public class Queen : Piece
  {
    public Queen(State state, Color color, int x, int y)
      : base(state, "q", color, x, y, true) {}
  }

  public class King : Piece
  {
    public King(State state, Color color, int x, int y)
      : base(state, "k", color, x, y, true) {}
  }
}
