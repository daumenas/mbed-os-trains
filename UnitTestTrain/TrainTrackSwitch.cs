using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace UnitTestTrain
{
    [TestClass]
    public class TrainTrackSwitch
    {

        [TestMethod]
        public void TestMethod2()
        {
            Assert.AreEqual("Inwards 1", send_command_switch(1, true));
        }

        [TestMethod]
        public void TestMethod3()
        {
            Assert.AreEqual("Inwards 2", send_command_switch(2, true));
        }

        [TestMethod]
        public void TestMethod4()
        {
            Assert.AreEqual("Inwards 3", send_command_switch(3, false));
        }

        [TestMethod]
        public void TestMethod5()
        {
            Assert.AreEqual("Inwards 4", send_command_switch(4, true));
        }


        [TestMethod]
        public void TestMethod6()
        {
            Assert.AreEqual("Outwards 1", send_command_switch(1, false));
        }

        [TestMethod]
        public void TestMethod7()
        {
            Assert.AreEqual("Outwards 2", send_command_switch(2, false));
        }

        [TestMethod]
        public void TestMethod8()
        {
            Assert.AreEqual("Outwards 3", send_command_switch(3, true));
        }

        [TestMethod]
        public void TestMethod9()
        {
            Assert.AreEqual("Outwards 4", send_command_switch(4, false));
        }

        [TestMethod]
        public void TestMethod10()
        {
            Assert.AreEqual("None", send_command_switch(5, false));
        }

        string send_command_switch(int sw, bool out_in)
        {
            switch (sw)
            {
                case 1:
                    if (out_in)
                    { // inwards (1 command)
                        return "Inwards 1";
                        //DCC_send_command(switch_address, 0x81, 1);
                    }
                    else
                    { // outwards (1 + idle)
                        return "Outwards 1";
                        //DCC_send_command(switch_address, 0x81, 1);
                        //DCC_send_command(switch_address, 0x80, 1);
                    }
                    break;
                case 2: // WARNING -> THIS SWITCH NEEDS AT LEAST 2s to be switched again!
                    if (out_in)
                    { // inwards (1 command)
                        return "Inwards 2";
                        //DCC_send_command(switch_address, 0x82, 1);
                    }
                    else
                    { // outwards (1 + idle)
                        return "Outwards 2";
                        //DCC_send_command(switch_address, 0x82, 1);
                        //DCC_send_command(switch_address, 0x80, 1);
                    }
                    break;
                case 3: // command reversed
                    if (out_in)
                    {
                        return "Outwards 3";
                        //DCC_send_command(switch_address, 0x84, 1);
                        //DCC_send_command(switch_address, 0x80, 1);
                    }
                    else
                    {
                        return "Inwards 3";
                        //DCC_send_command(switch_address, 0x84, 1);
                    }
                    break;
                case 4:
                    if (out_in)
                    { // inwards (1 command)
                        return "Inwards 4";
                        //DCC_send_command(switch_address, 0x88, 1);
                    }
                    else
                    {
                        return "Outwards 4";
                        // outwards (1 + idle)
                        //DCC_send_command(switch_address, 0x88, 1);
                        //DCC_send_command(switch_address, 0x80, 1);
                    }
                default:
                    return "None";
                    break;
            }
        }
    }
}
