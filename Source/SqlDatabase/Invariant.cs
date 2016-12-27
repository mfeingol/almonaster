using System;
using System.Diagnostics;

namespace Almonaster.Database.Sql
{
    public static class Invariant
    {
        public static void Assert(bool condition)
        {
            Assert(condition, String.Empty);
        }

        public static void Assert(bool condition, string message)
        {
            Debug.Assert(condition);
            if (!condition)
            {
                Environment.FailFast(message);
                throw new InvalidOperationException();
            }
        }
    }
}
