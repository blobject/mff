using System;
using System.Collections.Generic;
using Chesss.Controller;

namespace Chesss.Model
{
  public enum D { N, NE, E, SE, S, SW, W, NW }

  public class PieceGen
  {
    public static Piece
    Gen(string sym, C color, Place place)
    {
      if (sym == "p")
      {
        return new Pawn(color, place);
      }
      if (sym == "r")
      {
        return new Rook(color, place);
      }
      if (sym == "n")
      {
        return new Knight(color, place);
      }
      if (sym == "b")
      {
        return new Bishop(color, place);
      }
      if (sym == "q")
      {
        return new Queen(color, place);
      }
      if (sym == "k")
      {
        return new King(color, place);
      }
      return null;
    }
  }

  public abstract class Piece
  {
    public string Sym { get; set; }
    public C Color { get; set; }
    public Place Place { get; set; }
    public bool Inert { get; set; }
    public bool Captive { get; set; }

    public Piece() {}
    public Piece(string sym, C color, Place place)
    {
      this.Sym = sym;
      this.Color = color;
      this.Place = place;
      this.Inert = true;
      this.Captive = false;
    }

    public void
    Quicken()
    {
      this.Inert = false;
    }

    public void
    GetCaptured()
    {
      this.Captive = true;
    }

    public static WhoRet
    Step(D dir, Game game, Place here, out Place there)
    {
      there = here;
      if      (dir == D.N)  { there = here.N(); }
      else if (dir == D.NE) { there = here.NE(); }
      else if (dir == D.E)  { there = here.E(); }
      else if (dir == D.SE) { there = here.SE(); }
      else if (dir == D.S)  { there = here.S(); }
      else if (dir == D.SW) { there = here.SW(); }
      else if (dir == D.W)  { there = here.W(); }
      else if (dir == D.NW) { there = here.NW(); }
      if (there == null)
      {
        return WhoRet.Oob;
      }
      Piece that = game.At(there);
      if (that == null)
      {
        return WhoRet.Noone;
      }
      if (game.Turn != that.Color)
      {
        return WhoRet.Enemy;
      }
      if (game.Turn == that.Color)
      {
        return WhoRet.Friend;
      }
      return WhoRet.Error;
    }

    public abstract List<Place> Reach(Game game);
  }

  public class Pawn : Piece
  {
    public Pawn(C color, Place place) : base("p", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var reach = new List<Place>();
      Place place = this.Place;
      int file = place.File;
      int rank = place.Rank;
      D dir = D.N;
      if (this.Color == C.Black)
      {
        dir = D.S;
      }

      if (Step(dir, game, place, out place) == WhoRet.Noone)
      {
        reach.Add(place);
      }
      if (this.Inert
          && Step(dir, game, place, out place) == WhoRet.Noone)
      {
        place = new Place(file, rank + 2);
        if (this.Color == C.Black)
        {
          place = new Place(file, rank - 2);
        }
        reach.Add(place);
      }
      return reach;
    }
  }

  public class Rook : Piece
  {
    public Rook(C color, Place place) : base("r", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var reach = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in new D[] {D.N, D.E, D.S, D.W})
      {
        place = this.Place;
        while ((ret = Step(dir, game, place, out place)) == WhoRet.Noone)
        {
          reach.Add(place);
        }
        if (ret == WhoRet.Enemy)
        {
          reach.Add(place);
        }
      }
      return reach;
    }
  }

  public class Knight : Piece
  {
    public Knight(C color, Place place) : base("n", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var reach = new List<Place>();
      WhoRet ret;
      Place place;
      Place trace;
      D left;
      D right;
      foreach (var dir in new D[] {D.N, D.E, D.S, D.W})
      {
        place = this.Place;
        ret = Step(dir, game, place, out place);
        if (ret == WhoRet.Oob)
        {
          continue;
        }
        ret = Step(dir, game, place, out trace);
        if (ret == WhoRet.Oob)
        {
          continue;
        }
        if (dir == D.N || dir == D.S)
        {
          left = D.W;
          right = D.E;
        }
        else
        {
          left = D.N;
          right = D.S;
        }
        foreach (var dir2 in new D[] {left, right})
        {
          ret = Step(dir2, game, trace, out place);
          if (ret == WhoRet.Noone || ret == WhoRet.Enemy)
          {
            reach.Add(place);
          }
        }
      }
      return reach;
    }
  }

  public class Bishop : Piece
  {
    public Bishop(C color, Place place) : base("b", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var reach = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in new D[] {D.NE, D.SE, D.SW, D.NW})
      {
        place = this.Place;
        while ((ret = Step(dir, game, place, out place)) == WhoRet.Noone)
        {
          reach.Add(place);
        }
        if (ret == WhoRet.Enemy)
        {
          reach.Add(place);
        }
      }
      return reach;
    }
  }

  public class Queen : Piece
  {
    public Queen(C color, Place place) : base("q", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var rookCast = new Rook(this.Color, this.Place);
      var bishopCast = new Bishop(this.Color, this.Place);
      var reach = rookCast.Reach(game);
      foreach (var valid in bishopCast.Reach(game))
      {
        reach.Add(valid);
      }
      return reach;
    }
  }

  public class King : Piece
  {
    public King(C color, Place place) : base("k", color, place) {}

    public override List<Place>
    Reach(Game game)
    {
      var reach = new List<Place>();
      var threats = new HashSet<Place>();
      var safes = new List<Place>();
      WhoRet ret;
      Place place;
      foreach (var dir in new D[] {D.N, D.NE, D.E, D.SE,
                                   D.S, D.SW, D.W, D.NW})
      {
        place = this.Place;
        ret = Step(dir, game, place, out place);
        if (ret == WhoRet.Noone || ret == WhoRet.Enemy)
        {
          reach.Add(place);
        }
      }
      // TODO: capture but threatened

      // consider threats
      foreach (var enemy in game.Hand(game.Other()))
      {
        foreach (var threat in enemy.Reach(game))
        {
          threats.Add(threat);
        }
      }
      foreach (var valid in reach)
      {
        if (!threats.Contains(valid))
        {
          safes.Add(valid);
        }
      }

      return safes;
    }
  }
}
