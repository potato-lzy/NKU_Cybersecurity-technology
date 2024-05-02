import socket
pcaps = []

# 捕获
def sniffIpData(myip, choose):
    host_ip = myip  # 获取IP 即设置要捕获的网卡
    sniffer = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
    sniffer.bind((host_ip, 0))
    sniffer.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
    if choose == 'a':
        sniffer.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)
    print("[*] 正在嗅探数据.....")
    count = int(input("请输入您想嗅探的数据包个数："))
    while count:
        recv_data, addr = sniffer.recvfrom(1500)
        if recv_data:
            try:
                decodeIpData(recv_data)
            except:
                continue
        else:
            continue
        count -= 1
    sniffer.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)
    sniffer.close()
    return recv_data


# 提取协议名
def check_protocol(flag):
    if flag == 1:
        reasult = "ICMP"
    elif flag == 2:
        reasult = "IGMP"
    elif flag == 6:
        reasult = "TCP"
    elif flag == 17:
        reasult = "UDP"
    elif flag == 89:
        reasult = "OSPF"
    else:
        reasult = flag
    return reasult


# 解析
def decodeIpData(package):
    ip_data = {}
    # RFC791
    ip_data['version'] = package[0] >> 4  # 版本
    ip_data['headLength'] = package[0] & 0x0f  # 头长度  #& 按位与操作
    ip_data['DSField'] = package[1]  # Tos服务字段
    ip_data['totalLength'] = (package[2] << 8) + package[3]  # 总长度
    ip_data['identification'] = (package[4] << 8) + package[5]  # 标识分片
    ip_data['flag'] = package[6] >> 5  # 标志位
    ip_data['moreFragment'] = ip_data['flag'] & 1  # 标识后面可否还有分片
    ip_data['dontFragment'] = (ip_data['flag'] >> 1) & 1  # 能否分片 0可分片
    ip_data['fragmentOffset'] = ((package[6] & 0x1f) << 8) + package[7]  # 片偏移
    ip_data['TTL'] = package[8]  # 生存周期
    ip_data['protocol'] = check_protocol(package[9])  # 协议
    ip_data['headerCheckSum'] = (package[10] << 8) + package[11]  # 校验和
    # 以IP地址形式存储
    ip_data['sourceAddress'] = "%d.%d.%d.%d" % (package[12], package[13], package[14], package[15])
    ip_data['destinationAddress'] = "%d.%d.%d.%d" % (package[16], package[17], package[18], package[19])
    ip_data['options'] = []  # 可选项
    # 根据headerLength求出options
    if ip_data['headLength'] > 5:
        temp = 5
        while temp < ip_data['headLength']:
            ip_data['options'].append(package[temp * 4] + 0)
            ip_data['options'].append(package[temp * 4] + 1)
            ip_data['options'].append(package[temp * 4] + 2)
            ip_data['options'].append(package[temp * 4] + 3)
            temp += 1
    # 根据totalLength求出data
    ip_data['data'] = ""
    temp = ip_data['headLength'] * 4
    while temp < ip_data['totalLength']:
        ip_data['data'] += (str(hex(package[temp])).upper().replace("0X", ""))
        temp += 1
    pcaps.append(ip_data)
    return ip_data

# IP过滤
def ip_filter(choice):
    if choice == 'a':
        source = input("请输入源IP地址：")
        index = 0
        for pcap in pcaps:
            if pcap["sourceAddress"] == source:
                index += 1
                print("第{}条记录".format(index))
                for i in pcap.keys():
                    print("{}:{}".format(i, pcap[i]))
                print("\n")
        if index == 0:
            print("源IP输入错误或者无该记录！")
    if choice == 'b':
        dest = input("请输入目的IP地址：")
        index = 0
        for pcap in pcaps:
            if pcap["destinationAddress"] == dest:
                index += 1
                print("第{}条记录".format(index))
                for i in pcap.keys():
                    print("{}:{}".format(i, pcap[i]))
                print("\n")
        if index == 0:
            print("目的IP输入错误或者无该记录！")
    if choice == 'c':
        source = input("请输入源IP地址：")
        dest = input("请输入目的IP地址：")
        index = 0
        for pcap in pcaps:
            if pcap["sourceAddress"] == source and pcap["destinationAddress"] == dest:
                index += 1
                print("第{}条记录".format(index))
                for i in pcap.keys():
                    print("{}:{}".format(i, pcap[i]))
                print("\n")
        if index == 0:
            print("IP输入错误或者无该记录！")


# 过滤
def filter(options, target):
    if options == 1:
        print('''  
    --------a. 以混杂模式开始嗅探--------------------------
    --------b. 以普通模式开始嗅探------------------------
    --------c. 返回上个选项----------------------------
    ''')
        choose = input("输入选项：")
        if choose == 'c':
            return
        sniffIpData(myip=target, choose=choose)
        for pcap in pcaps:
            for i in pcap.keys():
                print("{}:{}".format(i, pcap[i]))
            print("\n")
    elif options == 2:
        while True:
            print('''  
    --------a. 按源ip地址筛选--------------------------
    --------b. 按目的IP地址筛选------------------------
    --------c. 同时按源ip地址和目的ip地址筛选---------
    --------d. 返回上个选项----------------------------
    ''')
            choice = input("输入选项：")
            ip_filter(choice)
            if choice == 'd':
                return
    elif options == 3:
        protocol = input("请输入协议名：")
        index = 0
        for pcap in pcaps:
            if pcap['protocol'] == protocol:
                index += 1
                print("第{}条记录".format(index))
                for i in pcap.keys():
                    print("{}:{}".format(i, pcap[i]))
                print("\n")
        if index == 0:
            print("协议名错误或无该记录！")

# 开始捕获
def sniffer(target):
    while True:
        print('''  
    --------1. 开始嗅探并输出嗅探结果（开始前请先选此选项）--------------
    --------2. 根据IP筛选----------------------------------------------------
    --------3. 根据协议筛选-------------------------------------------------
    --------4. 退出程序------------------------------------------------------
    ''')
        options = int(input("输入选项："))
        if options == 4:
            break
        filter(options, target)

target = input("请输入要嗅探网卡的IP：")
sniffer(target)
