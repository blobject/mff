using System;
using System.Collections.Generic;
using Xunit;
using Chesh.Controller;
using Chesh.Model;
using Chesh.Util;

namespace Test
{
  public class ModelTest
  {
    [Fact]
    public void GameInit()
    {
      Game o = new Game();
      Assert.NotNull(o);
      Assert.False(o.BlackDrew);
      Assert.False(o.WhiteDrew);
      Assert.Null(o.Forfeited);
      Assert.False(o.Over);
    }

    [Fact]
    public void StateInit()
    {
      State o = new State();
      Assert.NotNull(o);
      Assert.Equal(32, o.Live.Count);
      Assert.Empty(o.Dead);
      Assert.Empty(o.History);
      Assert.All(o.Live, entry =>
      {
        Assert.Contains(entry.Sym.ToLower(),
                        (new List<string>() {"o","p","r","n","b","q","k"}));
        Assert.True(entry.Color == Color.Black
                    || entry.Color == Color.White);
        Assert.True(entry.X >= 1 && entry.X <= 8);
        Assert.True(entry.Y >= 1 && entry.Y <= 8);
        Assert.True(entry.Inert);
      });
    }

    [Fact]
    public void StateMove()
    {
      State o = new State();
      Assert.Equal(Ret.Regular, o.Move(2, 2, 2, 3));
      Assert.Equal(Ret.BadTurn, o.Move(2, 3, 2, 4));
      Assert.Equal(Ret.Regular, o.Move(2, 7, 2, 5));
      Assert.Equal(Ret.InvalidMove, o.Move(1, 2, 1, 5));
    }
  }
}
