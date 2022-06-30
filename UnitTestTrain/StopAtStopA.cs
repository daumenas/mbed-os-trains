using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace UnitTestTrain
{
    [TestClass]
    public class StopAtStopA
    {
        [TestMethod]
        public void TestMethod1()
        {
            Assert.AreEqual("Train 1", StopAtStopAMethod(0x02));
        }

        [TestMethod]
        public void TestMethod2()
        {
            Assert.AreEqual("Train 2", StopAtStopAMethod(0x03));
        }

        public string StopAtStopAMethod (int train)
        {
            if (train == 0x02)
            {
                return "Train 1";
            }
            else
            {
                return "Train 2";
            }
        }
    }
}
