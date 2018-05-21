# cute

## a cute socket framework based epoll on linux with reactor pattern.

## feature
### portability
#### use C++ 11 feature(avoid use delete, new...)
### simplify
#### code is simplify enough to easy to use
### 
## so, it like a cute boy (or girl), working hard go on...

# turtorial

### server code:

    i32 main()
    {
        // init reactor
        cute_reactor reactor;
        reactor.init();

        // init acceptor
        auto acceptor = std::make_shared<cute_acceptor<echo_service_handler>>();
        cute_net_addr addr("0.0.0.0", 11000);
        acceptor->open(addr, &reactor);

        // go
        reactor.run_loop();	
    }

### client code:

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

# compile it and run sample
#### cmake -DCMAKE_BUILD_TYPE=Debug .
#### make 
#### ./sample/server &
#### ./sample/client 127.0.0.1 11000

# enjoy it ^-^

