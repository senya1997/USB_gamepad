using System;
using System.IO.Ports;
using System.Text;

namespace gp_test
{
    class Program
    {
        static SerialPort port = new SerialPort();

        static byte[] buf = new byte[6]; // key1, key2, RjX, RjY, LjX, LjY
        static byte[] buf_temp = new byte[3];

        static byte j = 0;
        static byte k = 0;

        const string format = "X";

        static void Main(string[] args)
        {
            int num;
            bool is_int;

            string[] all_port = SerialPort.GetPortNames();
            string ans;
            string str_temp;
            
            buf_temp[0] = 0;
            buf_temp[1] = 0;
            buf_temp[2] = 0;

            Console.WriteLine("Choose port:\n");
            for (int i = 0; i < all_port.Length; i++)
                Console.WriteLine("[" + i + "] " + all_port[i].ToString());
            
            READ_NUM:   str_temp = Console.ReadLine();
            is_int = Int32.TryParse(str_temp, out num);
            if (!is_int) goto READ_NUM;
            
            try
            {
                port.PortName = all_port[num];
                port.BaudRate = 9600;
                port.DataBits = 8;
                port.Parity = Parity.None;
                port.StopBits = StopBits.One;
                port.ReadTimeout = 100;
                port.WriteTimeout = 100;
                port.Handshake = Handshake.None;
                port.DataReceived += Port_DataReceived;
                port.Open();
                Console.Clear();
            }
            catch (Exception e)
            {
                Console.WriteLine("\n\t***** Unable to open port *****\nError: " + e.ToString());
                Console.WriteLine("\nChoose port again");
                goto READ_NUM;
            }

REPEAT:     ans = Console.ReadLine();
            if (ans == "ex")
            {
                port.Close();

                Console.Clear();
                Console.WriteLine("Press any key...");
                Console.ReadKey();
            }
            else goto REPEAT;
        }

        private static void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            //                  0xFF                    0x73                0x5A
            if ((buf_temp[0] == 255) & (buf_temp[1] == 115) & (buf_temp[2] == 90))
            {
                buf[j] = Convert.ToByte(port.ReadByte());

                if (j == 5)
                {
                    j = 0;

                    buf_temp[0] = 0;
                    buf_temp[1] = 0;
                    buf_temp[2] = 0;
                }
                else j++;
            }
            else
            {
                buf_temp[k] = Convert.ToByte(port.ReadByte());

                if (k == 2) k = 0;
                else k++;
            }
            
            Console.SetCursorPosition(0, 0);
            Console.WriteLine(buf[0].ToString(format) + "  " + 
                              buf[1].ToString(format) + "  " + 
                              buf[2].ToString(format) + "  " + 
                              buf[3].ToString(format) + "  " + 
                              buf[4].ToString(format) + "  " +
                              buf[5].ToString(format));
        }
    }
}
