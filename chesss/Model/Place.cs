using System;
using System.Collections.Generic;

namespace Chesss.Model
{
  public class Place : IEquatable<Place>
  {
    private int file;
    private int rank;

    public Place(int file, int rank)
    {
      this.file = file;
      this.rank = rank;
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
      return this.file == place.File() && this.rank == place.Rank();
    }

    public override int GetHashCode()
    {
      unchecked
      {
        int hash = 17;
        hash = hash * 23 + this.file.GetHashCode();
        hash = hash * 23 + this.rank.GetHashCode();
        return hash;
      }
    }

    public override string
    ToString()
    {
      return string.Format("{0}{1}", (char) (this.file + 'a' - 1), this.rank);
    }

    public int
    File()
    {
      return this.file;
    }

    public int
    Rank()
    {
      return this.rank;
    }

    public Place
    N()
    {
      int rank = this.rank + 1;
      if (rank > 8)
      {
        return null;
      }
      return new Place(this.file, rank);
    }

    public Place
    NE()
    {
      int file = this.file + 1;
      int rank = this.rank + 1;
      if (file > 8 || rank > 8)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    E()
    {
      int file = this.file + 1;
      if (file > 8)
      {
        return null;
      }
      return new Place(file, this.rank);
    }

    public Place
    SE()
    {
      int file = this.file + 1;
      int rank = this.rank - 1;
      if (file > 8 || rank < 1)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    S()
    {
      int rank = this.rank - 1;
      if (rank < 1)
      {
        return null;
      }
      return new Place(this.file, rank);
    }

    public Place
    SW()
    {
      int file = this.file - 1;
      int rank = this.rank - 1;
      if (file < 1 || rank < 1)
      {
        return null;
      }
      return new Place(file, rank);
    }

    public Place
    W()
    {
      int file = this.file - 1;
      if (file < 1)
      {
        return null;
      }
      return new Place(file, this.rank);
    }

    public Place
    NW()
    {
      int file = this.file - 1;
      int rank = this.rank + 1;
      if (file < 1 || rank > 8)
      {
        return null;
      }
      return new Place(file, rank);
    }
  }
}
