using System;
using System.IO.Ports;
using System.Text;

namespace gp_test
{
    class Program
    {
        static SerialPort port = new SerialPort();

        static byte[] buf = new byte[8];

        static byte j = 0;
        static bool flag_st_rec = false;

        const string format = "X";

        static void Main(string[] args)
        {
            int num;
            bool is_int;

            string[] all_port = SerialPort.GetPortNames();
            string ans;
            string str_temp;
            

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
            byte buf_temp = Convert.ToByte(port.ReadByte());

            if(buf_temp == 115) // 0x73
            {
                buf[0] = buf_temp;
                flag_st_rec = true;
                j++;
            }

            if(flag_st_rec)
            {
                buf[j] = Convert.ToByte(port.ReadByte());

                if (j == 7)
                {
                    j = 0;
                    flag_st_rec = false;
                }
                else j++;
            }
            
            Console.SetCursorPosition(0, 0);
            Console.WriteLine(buf[0].ToString(format) + "  " + 
                              buf[1].ToString(format) + "  " + 
                              buf[2].ToString(format));

            Console.WriteLine(buf[3].ToString(format) + "  " + 
                              buf[4].ToString(format) + "  " + 
                              buf[5].ToString(format) + "  " +
                              buf[6].ToString(format) + "  " + 
                              buf[7].ToString(format));

            /*
            var port_sender = (SerialPort)sender;
            try
            {
                //  узнаем сколько байт пришло
                int buferSize = port.BytesToRead;
                for (int i = 0; i < buferSize; ++i)
                {
                    //  читаем по одному байту
                    byte bt = (byte)port.ReadByte();
                    //  если встретили начало кадра (0xFF) - начинаем запись в _bufer
                    if (0xFF == bt)
                    {
                        _stepIndex = 0;
                        _startRead = true;
                        //  раскоментировать если надо сохранять этот байт
                        //_bufer[_stepIndex] = bt;
                        //++_stepIndex;
                    }
                    //  дописываем в буфер все остальное
                    if (_startRead)
                    {
                        _bufer[_stepIndex] = bt;
                        ++_stepIndex;
                    }
                    //  когда буфер наполнлся данными
                    if (_stepIndex == DataSize && _startRead)
                    {
                        //  по идее тут должны быть все ваши данные.

                        //  .. что то делаем ...
                        //  var item = _bufer[7];

                        _startRead = false;
                    }
                }
            }
            catch { }
            */
        }
    }
}
