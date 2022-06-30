using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace UnitTestTrain
{
    [TestClass]
    public class LightSignals
    {
        [TestMethod]
        public void TestMethod1()
        {
            Assert.AreEqual(0, readSignal(0, 000000011, 0000000000));
        }

        [TestMethod]
        public void TestMethod2()
        {
            Assert.AreEqual(3, readSignal(0, 11111100, 000000000));
        }

        [TestMethod]
        public void TestMethod3()
        {
            Assert.AreEqual(3, readSignal(4, 11111100, 000000000));
        }

        [TestMethod]
        public void TestMethod4()
        {
            Assert.AreEqual(2, readSignal(4, 01111100, 11111101));
        }

        [TestMethod]
        public void TestMethod5()
        {
            Assert.AreEqual(1, readSignal(4, 01111100, 11111110));
        }

        public int readSignal(int signal, int current_OLATA, int current_OLATB)
        {
            bool g_bit = false;
            bool r_bit = false;
            switch (signal)
            {
                case 0:
                    g_bit = (current_OLATA & (1 << signal * 2)) != 0;
                    r_bit = (current_OLATA & (1 << signal * 2 + 1)) != 0;
                    break;
                case 1:
                    g_bit = (current_OLATA & (1 << signal * 2)) != 0;
                    r_bit = (current_OLATA & (1 << signal * 2 + 1)) != 0;
                    break;
                case 2:
                    g_bit = (current_OLATA & (1 << signal * 2)) != 0;
                    r_bit = (current_OLATA & (1 << signal * 2 + 1)) != 0;
                    break;
                case 3:
                    g_bit = (current_OLATA & (1 << signal * 2)) != 0;
                    r_bit = (current_OLATA & (1 << (signal * 2 + 1))) != 0;
                    break;
                case 4:
                    g_bit = (current_OLATB & (1 << (signal - 4) * 2)) != 0;
                    r_bit = (current_OLATB & (1 << ((signal - 4) * 2 + 1))) != 0;
                    break;
                case 5:
                    g_bit = (current_OLATB & (1 << (signal - 4) * 2)) != 0;
                    r_bit = (current_OLATB & (1 << ((signal - 4) * 2 + 1))) != 0;
                    break;
                case 6:
                    g_bit = (current_OLATB & (1 << (signal - 4) * 2)) != 0;
                    r_bit = (current_OLATB & (1 << ((signal - 4) * 2 + 1))) != 0;
                    break;
                case 7:
                    g_bit = (current_OLATB & (1 << (signal - 4) * 2)) != 0;
                    r_bit = (current_OLATB & (1 << ((signal - 4) * 2 + 1))) != 0;
                    break;
                default:
                    break;
            }
            // 0 is ON and 1 is OFF
            if (g_bit)
            {
                if (r_bit)
                {
                    return 0;
                }
                else
                {
                    return 2;
                }
            }
            else
            {
                if (r_bit)
                {
                    return 1;
                }
                else
                {
                    return 3;
                }
            }
        }
    }
}
