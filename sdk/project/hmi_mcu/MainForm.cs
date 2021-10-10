using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Windows.Forms;

namespace hmi_mcu
{
    public partial class MainForm : Form
    {
        const int maxBufSize = 4096;
        const int minDataLen = 10;
        
        delegate void ProcessDataCallback(byte[] data);

        SerialPort comPort = new SerialPort();
        byte[] buffer = new byte[maxBufSize];
        int bufSize = 0;

        enum ExternalEventType
        {
            EXTERNAL_TAP_POWER,         ///< tap power
            EXTERNAL_TAP_AUTO,          ///< tap auto
            EXTERNAL_TAP_COOL,          ///< tap cool
            EXTERNAL_TAP_DRY,           ///< tap dry
            EXTERNAL_TAP_FAN,           ///< tap fan
            EXTERNAL_TAP_HEAT,          ///< tap heat
            EXTERNAL_DRAG_TEMP_UP,      ///< drag temperature up
            EXTERNAL_DRAG_TEMP_DOWN,    ///< drag temperature down
            EXTERNAL_TAP_WIND_HIGH,     ///< tap wind high
            EXTERNAL_TAP_WIND_MID,      ///< tap wind middle
            EXTERNAL_TAP_WIND_LOW,      ///< tap wind low
            EXTERNAL_TAP_WIND_AUTO,     ///< tap wind auto
            EXTERNAL_TAP_HOME_PAGE_UP,
            EXTERNAL_TAP_HOME_PAGE_DOWN,
            EXTERNAL_TAP_AIRCONDITIONER,
            EXTERNAL_TAP_MULTIMEDIA
        };

        public MainForm()
        {
            InitializeComponent();
            InitializePorts();
            paramListBox.SetSelected(0, true);
        }

        private void InitializePorts()
        {
            string[] portNames = SerialPort.GetPortNames();
            foreach (string name in portNames)
            {
                portComboBox.Items.Add(name);
            }
            comPort.DataReceived += new SerialDataReceivedEventHandler(comPort_DataReceived);
        }

        public ushort SwapBytes(ushort x)
        {
            return (ushort)((ushort)((x & 0xff) << 8) | ((x >> 8) & 0xff));
        }

        private void ProcessData(byte[] data)
        {
            if (bufSize > 0)
            {
                int size = data.Length;

                if (size > maxBufSize - bufSize)
                    size = maxBufSize - bufSize;

                Array.Copy(data, 0, buffer, bufSize, size);
                bufSize += size;
            }
            else
            {
                for (int i = 0; i < data.Length; ++i)
                {
                    if (data[i] == 0xEE)
                    {
                        int size = data.Length - i;

                        if (size > maxBufSize - bufSize)
                            size = maxBufSize - bufSize;

                        Array.Copy(data, i, buffer, bufSize, size);
                        bufSize += size;

                        break;
                    }
                }
            }

            if (bufSize >= minDataLen && buffer[0] == 0xEE)
            {
                ushort cmd = BitConverter.ToUInt16(buffer, 1);

                cmd = SwapBytes(cmd);

                if (cmd >= 0)
                {
                    ExternalEventType exttype = (ExternalEventType)cmd;

                    byte[] content = new byte[3];
                    Array.Copy(buffer, 1, content, 0, 3);
                    ushort crc = Crc16(content);

                    ushort crc16 = BitConverter.ToUInt16(buffer, 4);
                    crc16 = SwapBytes(crc16);

                    if (crc == crc16)
                    {
                        int param = (int)buffer[3];

                        this.recvTextBox.Text += commandListBox.Items[cmd].ToString() + " ";
                        switch (exttype)
                        {
                            case ExternalEventType.EXTERNAL_TAP_POWER:
                            case ExternalEventType.EXTERNAL_TAP_AUTO:
                            case ExternalEventType.EXTERNAL_TAP_COOL:
                            case ExternalEventType.EXTERNAL_TAP_DRY:
                            case ExternalEventType.EXTERNAL_TAP_FAN:
                            case ExternalEventType.EXTERNAL_TAP_HEAT:
                            case ExternalEventType.EXTERNAL_TAP_WIND_HIGH:
                            case ExternalEventType.EXTERNAL_TAP_WIND_MID:
                            case ExternalEventType.EXTERNAL_TAP_WIND_LOW:
                            case ExternalEventType.EXTERNAL_TAP_WIND_AUTO:
                            case ExternalEventType.EXTERNAL_TAP_AIRCONDITIONER:
                            case ExternalEventType.EXTERNAL_TAP_MULTIMEDIA:
                                {
                                    this.recvTextBox.Text += paramListBox.Items[param].ToString() + "\r\n";
                                }
                                break;

                            case ExternalEventType.EXTERNAL_DRAG_TEMP_UP:
                            case ExternalEventType.EXTERNAL_DRAG_TEMP_DOWN:
                            case ExternalEventType.EXTERNAL_TAP_HOME_PAGE_UP:
                            case ExternalEventType.EXTERNAL_TAP_HOME_PAGE_DOWN:
                                {
                                    this.recvTextBox.Text += param.ToString() + "\r\n";
                                }
                                break;
                        }
                        this.recvTextBox.SelectionStart = this.recvTextBox.TextLength;
                        this.recvTextBox.ScrollToCaret();
                    }
                }
                bufSize = 0;
            }
            //this.recvTextBox.Text += System.Text.Encoding.UTF8.GetString(data);
        }

        void comPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (comPort.BytesToRead > 0)
            {
                int len = comPort.BytesToRead;
                byte[] buf = new byte[len];
                
                comPort.Read(buf, 0, len);

                this.BeginInvoke(new ProcessDataCallback(ProcessData), new object[] { buf });
            }
        }

        private void portStateButton_Click(object sender, EventArgs e)
        {
            if (portStateButton.Text == "Connect")
            {
                string port = portComboBox.SelectedItem as string;
                if (port != null && port != String.Empty)
                {
                    portStateButton.Text = "Disconnect";
                    comPort.PortName = port;
                    comPort.BaudRate = 115200;
                    comPort.DataBits = 8;
                    comPort.StopBits = StopBits.One;
                    comPort.Handshake = Handshake.None;
                    comPort.Parity = Parity.None;
                    comPort.Open();
                }
            }
            else if (portStateButton.Text == "Disconnect")
            {
                portStateButton.Text = "Connect";
                comPort.Close();
            }
        }

        public ushort Crc16(byte[] data)
        {
            ushort crc = 0x0000;

            for (int i = 0; i < data.Length; i++)
            {
                crc ^= (ushort)(data[i] << 8);
                for (int j = 0; j < 8; j++)
                {
                    if ((crc & 0x8000) > 0)
                        crc = (ushort)((crc << 1) ^ 0x8408);
                    else
                        crc <<= 1;
                }
            }
            return crc;
        }

        private int GetParamSize(ExternalEventType exttype)
        {
            switch (exttype)
            {
                case ExternalEventType.EXTERNAL_TAP_POWER:
                    return 1;

                default:
                    return 0;
            }
        }

        private void execButton_Click(object sender, EventArgs e)
        {
            if (portStateButton.Text == "Connect")
                return;

            int cmd = commandListBox.SelectedIndex;

            if (cmd >= 0)
            {
                ExternalEventType exttype = (ExternalEventType)cmd;
                int paramSize = GetParamSize(exttype);
                int size = paramSize + 9;
                byte[] data = new byte[size];

                data[0] = 0xEE;
                data[1] = 0x00;
                data[2] = (byte)cmd;

                switch (exttype)
                {
                    case ExternalEventType.EXTERNAL_TAP_POWER:
                        {
                            int param = paramListBox.SelectedIndex;
                            if (param >= 0)
                                data[3] = (byte)param;
                        }
                        break;
                }

                byte[] content = new byte[2 + paramSize];
                Array.Copy(data, 1, content, 0, 2 + paramSize);

                ushort crc = Crc16(content);
                crc = SwapBytes(crc);

                byte[] crc16 = BitConverter.GetBytes(crc);
                Array.Copy(crc16, 0, data, size - 6, 2);

                data[size - 4] = 0xFF;
                data[size - 3] = 0xFC;
                data[size - 2] = 0xFF;
                data[size - 1] = 0xFF;

                comPort.Write(data, 0, data.Length);
            }
        }

    }
}
