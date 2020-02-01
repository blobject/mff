using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public class Place : IEquatable<Place>
  {
    public int File { get; set; }
    public int Rank { get; set; }

    public Place(int file, int rank)
    {
      this.File = file;
      this.Rank = rank;
    }

    public override bool Equals(object o)
    {
      return this.Equals(o as Place);
    }

    public bool Equals(Place place)
    {
      if (Object.ReferenceEquals(place, null))
      {
        return false;
      }
      if (Object.ReferenceEquals(this, place))
      {
        return true;
      }
      return this.File == place.File && this.Rank == place.Rank;
    }

    public override int GetHashCode()
    {
      unchecked
      {
        int hash = 17;
        hash = hash * 23 + this.File.GetHashCode();
        hash = hash * 23 + this.Rank.GetHashCode();
        return hash;
      }
    }

    public override string
    ToString()
    {
      return string.Format("{0}{1}", (char) (this.File + 'a' - 1), this.Rank);
    }

    public Place
    N()
    {
      int rank = this.Rank + 1;
      if (rank > 8)
      {
        return null;
      }
      return new Place(this.File, rank);
    }

    public Place
    NE()
    {
      int file = this.File + 1;
      int rank = this.Rank + 1;
      if (file > 8 || rank > 8)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    E()
    {
      int file = this.File + 1;
      if (file > 8)
      {
        return null;
      }
      return new Place(file, this.Rank);
    }

    public Place
    SE()
    {
      int file = this.File + 1;
      int rank = this.Rank - 1;
      if (file > 8 || rank < 1)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    S()
    {
      int rank = this.Rank - 1;
      if (rank < 1)
      {
        return null;
      }
      return new Place(this.File, rank);
    }

    public Place
    SW()
    {
      int file = this.File - 1;
      int rank = this.Rank - 1;
      if (file < 1 || rank < 1)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    W()
    {
      int file = this.File - 1;
      if (file < 1)
      {
        return null;
      }
      return new Place(file, this.Rank);
    }

    public Place
    NW()
    {
      int file = this.File - 1;
      int rank = this.Rank + 1;
      if (file < 1 || rank > 8)
      {
        return null;
      }
      return new Place(file, rank);
    }
  }
}
