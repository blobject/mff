using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public class Piece
  {
    private P sym;
    private C color;
    private Place place;
    private bool inert;

    public Piece(P sym, C color, Place place)
    {
      this.sym = sym;
      this.color = color;
      this.place = place;
      this.inert = true;
    }

    public P
    Sym()
    {
      return this.sym;
    }

    public C
    Color()
    {
      return this.color;
    }

    public Place
    Place()
    {
      return this.place;
    }

    public bool
    Inert()
    {
      return this.inert;
    }

    public void
    Quicken()
    {
      this.inert = false;
    }
  }
}
