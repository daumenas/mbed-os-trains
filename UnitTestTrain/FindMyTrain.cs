using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace UnitTestTrain
{
    [TestClass]
    public class FindMyTrain
    {
        [TestMethod]
        public void TestMethod1()
        {
            Assert.AreEqual("A triggered", findMyTrain(1, 0));
        }

        [TestMethod]
        public void TestMethod2()
        {
            Assert.AreEqual("B triggered", findMyTrain(0,1));
        }

        [TestMethod]
        public void TestMethod3()
        {
            Assert.AreEqual("None", findMyTrain(0, 0));
        }

        public string findMyTrain(int loopATrigger, int loopBTrigger)
        {
            if (loopATrigger == 1)
            {
                loopATrigger = 0;
                return "A triggered";

            }
            else if (loopBTrigger == 1)
            {
                loopBTrigger = 0;
                return "B triggered";
            }

            return "None";
        }
    }
}
