# cute

## a cute socket IO framework based linux epoll with reactor pattern.
---
### Features
#### 1  encapsulation: socket IO API, linux epoll API
#### 2 defines the connector, acceptor and service three roles, connector is used to actively establish a TCP connection; acceptor passively receives and establishes a TCP connection, service is defined as a service logic module (based on the POSA2 acceptor-connector pattern)
#### 3 provides timer framework (event management and timing trigger) (based on timer-wheel)
#### 4 provides fixed byte size memory pool management
#### 5 provides application layer information encapsulation, can be used as send and receive buffers, support serialization and deserialization (continuous to be optimization)
---
### characteristic
Using the new features of C++ 11(14) (prefect-forward, universal reference, move-semantic, smart-pointer)
#### 1 Concurrency: 
##### support multi-threaded concurrent socket io event detection and dispatch processing
#### 2 Robustness:
##### 2.1 Avoid misusing native pointers and using smart pointers wherever possible
##### 2.2 Utilize the compiler deduct feature to reduce the hard coding of the type in the code
#### 3 High performance:
##### 3.1 Avoid unnecessary object copy
##### 3.2 High-frequency and variable-length memory allocation and recovery through the memory pool
##### 3.3 Always use non-blocking IO operating modes

---
### basic-server-client sample:

#### client code:

    i32 main(i32 argc, char* argv[])
    {
        if (argc < 3)
	    {
            std::cout << "usage: test_cute_connector <host> <port>" << std::endl;
            return 0;
	    }

        // init reactor
        cute_reactor reactor;
        reactor.init();

        // init connector and try connect to server
        auto connector = std::make_shared<cute_connector<my_service_handler>>();
        cute_net_addr addr(argv[1], std::atoi(argv[2]));
        connector->connect(addr, &reactor);
	
        // go
        reactor.run_loop();	
    }

#### server code:

    i32 main()
    {
        // init reactor
        cute_reactor reactor;
        reactor.init();

        // init acceptor
        auto acceptor = std::make_shared<cute_acceptor<echo_service_handler>>();
        cute_net_addr addr("0.0.0.0", 11000);
        acceptor->open(addr, &reactor);

        // run reactor event demultiplex and dispatch
        reactor.run_loop();	
    }

#### compile it and run sample
##### cd <setup_folder>/cute
##### cmake -DCMAKE_BUILD_TYPE=Debug .
##### make 
##### ./sample/server &
##### ./sample/client 127.0.0.1 11000

### file_translate sample:

#### file_translate_client:

	i32 main(i32 argc, char* argv[])
	{
		if (argc < 4)
		{
			std::cout << "usage: client <host> <port> <file_path>" << std::endl;
			return 0;
		}

		g_mem_pool_ptr = new cute_mem_pool();
		g_mem_pool_ptr->init(ALLOC_NODE_SIZE, ALLOC_NODE_COUNT);

		cute_reactor reactor;
		reactor.init();
		auto connector = std::make_shared<file_cute_connector>(argv[3]);
		cute_net_addr addr(argv[1], std::atoi(argv[2]));
		connector->connect(addr, &reactor);
		reactor.run_loop();
		return 0;
	
#### file_translate_server
	i32 main()
	{
		g_mem_pool_ptr = new cute_mem_pool();
		g_mem_pool_ptr->init(ALLOC_NODE_SIZE, ALLOC_NODE_COUNT);

		cute_reactor reactor;	
		reactor.init();
		auto acceptor = std::make_shared<cute_acceptor<file_translate_handler>>();
		cute_net_addr addr("0.0.0.0", 11000);
		acceptor->open(addr, &reactor);

		// try multiple thread event demultiplexed and dispatch
		std::thread t1 = std::thread([&reactor]()
		{
			reactor.run_loop();
		});

		std::thread t2 = std::thread([&reactor]()
		{
			reactor.run_loop();
		});

		t1.join();
		t2.join();

		return 0;
	}
	
#### compile it and run sample
##### cd <setup_folder>/cute
##### cmake -DCMAKE_BUILD_TYPE=Debug .
##### make 
##### ./sample/file_translate_server &
##### ./sample/file_translate_client 127.0.0.1 11000 <to_send_file_name>
	
# enjoy it ^-^

