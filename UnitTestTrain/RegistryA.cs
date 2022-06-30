using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace UnitTestTrain
{
    [TestClass]
    public class RegistryA
    {
        [TestMethod]
        public void TestMethod1()
        {
        }

        [TestMethod]
        public void ValidatePass1()
        {
            int switchHit = 0xfb;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xfb;
            int train2DetectorPreviousHit = 0x7e;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass2()
        {
            int switchHit = 0xf7;
            int train1DetectorPreviousHit = 0xbf;
            int train1DetectorNextHit = 0xf7;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass3
            ()
        {
            int switchHit = 0x7f;
            int train1DetectorPreviousHit = 0xfd;
            int train1DetectorNextHit = 0x7f;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass4()
        {
            int switchHit = 0xbf;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xbf;
            int train2DetectorPreviousHit = 0xf7;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass5()
        {
            int switchHit = 0xfe;
            int train1DetectorPreviousHit = 0xdf;
            int train1DetectorNextHit = 0xfe;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass6()
        {
            int switchHit = 0xfd;
            int train1DetectorPreviousHit = 0xfe;
            int train1DetectorNextHit = 0xfd;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 1", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass7()
        {
            int switchHit = 0xfb;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xfb;
            int train1DetectorPreviousHit = 0x7e;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass8()
        {
            int switchHit = 0xf7;
            int train2DetectorPreviousHit = 0xbf;
            int train2DetectorNextHit = 0xf7;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass9
            ()
        {
            int switchHit = 0x7f;
            int train2DetectorPreviousHit = 0xfd;
            int train2DetectorNextHit = 0x7f;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass10()
        {
            int switchHit = 0xbf;
            int train2DetectorPreviousHit = 0x7f;
            int train2DetectorNextHit = 0xbf;
            int train1DetectorPreviousHit = 0xf7;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass11()
        {
            int switchHit = 0xfe;
            int train2DetectorPreviousHit = 0xdf;
            int train2DetectorNextHit = 0xfe;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }
        [TestMethod]
        public void ValidatePass12()
        {
            int switchHit = 0xfd;
            int train2DetectorPreviousHit = 0xfe;
            int train2DetectorNextHit = 0xfd;
            int train1DetectorPreviousHit = 0x7f;
            int train1DetectorNextHit = 0xfe;

            int missedSensorIndex = 0;

            Assert.AreEqual("Train 2", CheckSwitchCase(switchHit, train1DetectorPreviousHit, train2DetectorPreviousHit, train1DetectorNextHit, train2DetectorNextHit, ref missedSensorIndex));
        }

        public string CheckSwitchCase(int switchHit, int train1DetectorPreviousHit, int train2DetectorPreviousHit, int train1DetectorNextHit, int train2DetectorNextHit, ref int indexMissedSensors)
        {
            switch (switchHit)
            {
                case 0xfe:
                    if (train1DetectorPreviousHit == 0xdf || train1DetectorPreviousHit == 0xef)
                    {
                        if (train1DetectorNextHit != 0xfe)
                        {
                            indexMissedSensors++;
                        }

                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0xdf || train2DetectorPreviousHit == 0xef)
                    {
                        if (train2DetectorNextHit != 0xfe)
                        {
                            indexMissedSensors++;
                        }

                        return "Train 2";
                    }
                    break;
                case 0xfd:
                    if (train1DetectorPreviousHit == 0xfe || train1DetectorNextHit == 0xfd)
                    {
                        if (train1DetectorNextHit != 0xfd)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0xfe || train1DetectorNextHit == 0xfd)
                    {
                        if (train2DetectorNextHit != 0xfd)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 2";
                    }
                    break;
                case 0xfb:
                    if (train1DetectorPreviousHit == 0x7f || train1DetectorPreviousHit == 0xfd)
                    {
                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0x7f || train2DetectorPreviousHit == 0xfd)
                    {
                        return "Train 2";
                    }
                    break;
                case 0xf7:
                    if (train1DetectorPreviousHit == 0xbf)
                    {
                        if (train1DetectorNextHit != 0xf7)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0xbf)
                    {
                        if (train2DetectorNextHit != 0xf7)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 2";
                    }
                    break;
                case 0xef:
                    indexMissedSensors = 4;
                    break;
                case 0xdf:
                    // doesn't exist
                    break;
                case 0xbf:
                    if (train1DetectorPreviousHit == 0x7f)
                    {
                        if (train1DetectorNextHit != 0xbf)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0x7f)
                    {
                        if (train2DetectorNextHit != 0xbf)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 2";
                    }
                    break;
                case 0x7f:
                    if (train1DetectorPreviousHit == 0xfd)
                    {
                        if (train1DetectorNextHit != 0x7f)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 1";
                    }
                    else if (train2DetectorPreviousHit == 0xfd)
                    {
                        if (train2DetectorNextHit != 0x7f)
                        {
                            indexMissedSensors++;
                        }
                        return "Train 2";
                    }
                    break;
                default:
                    return "Error";
                    break;
            }

            return "Error";
        }
    }
}
