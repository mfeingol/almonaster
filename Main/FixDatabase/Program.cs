using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Transactions;

namespace FixDatabase
{
    class Program
    {
        static void Main(string[] args)
        {
            //const string DSN = @"Data Source=.\SQLExpress;Integrated Security=SSPI;Initial Catalog=Almonaster";
            const string DSN = @"Server=(localdb)\MSSQLLocalDB;Integrated Security=true;Initial Catalog=Almonaster;AttachDbFileName=C:\Users\mfeingol\Documents\Build\Almonaster\Main\Drop\x64\Debug\site\Almonaster.mdf";

            using (TransactionScope ts = new TransactionScope())
            {
                using (SqlConnection conn = new SqlConnection(DSN))
                {
                    conn.Open();

                    var updates = new List<Tuple<long, string[], string[]>>();
                    using (SqlCommand cmd = conn.CreateCommand())
                    {
                        cmd.CommandText = "SELECT [Id], [Winners], [Losers] FROM [Almonaster].[dbo].[SystemLatestGames]";
                        using (SqlDataReader reader = cmd.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                long id = reader.GetInt64(0);
                                string[] winners = reader.GetString(1).Split(',');
                                string[] losers = reader.GetString(2).Split(',');

                                updates.Add(new Tuple<long, string[], string[]>(id, TrimStrings(winners), TrimStrings(losers)));
                            }
                        }

                        //foreach (var update in updates)
                        //{
                        //    RewriteWinnersLosers(conn, update.Item1, update.Item2, update.Item3);
                        //}
                    }
                }
                //ts.Complete();
            }
        }

        static string[] TrimStrings(string[] strings)
        {
            string[] trimmed = new string[strings.Length];
            for(int i = 0; i < strings.Length; i ++)
            {
                trimmed[i] = strings[i].Trim();
            }
            return trimmed;
        }

        static void RewriteWinnersLosers(SqlConnection conn, long id, string[] winners, string[] losers)
        {
            string loserStr = String.Empty;
            foreach (string loser in losers)
            {
                bool isWinner = false;
                foreach (string winner in winners)
                {
                    if (loser == winner)
                    {
                        isWinner = true;
                        break;
                    }
                }
                if (!isWinner)
                {
                    if (!String.IsNullOrEmpty(loserStr))
                    {
                        loserStr += ", ";
                    }
                    loserStr += loser;
                }
            }

            using (SqlCommand cmd = conn.CreateCommand())
            {
                cmd.CommandText = "UPDATE [Almonaster].[dbo].[SystemLatestGames] SET [Losers] = @p0 WHERE [Id] = @p1";
                cmd.Parameters.AddWithValue("@p0", loserStr);
                cmd.Parameters.AddWithValue("@p1", id);
                int x = cmd.ExecuteNonQuery();
            }
        }
    }
}
