#!/usr/bin/env bash

function trace_help() {
    echo "Options:"
    echo "--ncaplay / -n IP RATE: Connect to a stream over TCP and pipe into aplay"
    echo "--playto / -p IP FILENAME : Send raw signed int 32, 16kHz audio"
    echo "--udpcli  / -u IP : Connect to CLI"
    echo "--thruput / -t IP : Run the throughput test"
    echo "--tlsechosrv / -e : Run an echo server with TLS"
    echo "--tlsechocli / -c : Connect to an echo server with TLS"
    return
}

if [ $# == 1 ]
then
    if [ "$1" == "--help" ] || [ "$1" == "-h" ]
    then
        trace_help
    elif [ "$1" == "--tlsechosrv" ] || [ "$1" == "-e" ]
    then
        ncat -e /bin/cat -k -4 -l 25565 --ssl --ssl-cert ./filesystem_support/echo_client_certs/server.pem  --ssl-key ./filesystem_support/echo_client_certs/server.key
    elif [ "$1" == "--tlsechocli" ] || [ "$1" == "-c" ]
    then
        ncat 10.0.0.142 7777 --ssl --ssl-cert ./filesystem_support/board_server_certs/client.pem --ssl-key ./filesystem_support/board_server_certs/client.key

    fi
elif [ $# == 2 ]
then
    if [ "$1" == "--udpcli" ] || [ "$1" == "-u" ]
    then
        ncat -u $2 5432
    elif [ "$1" == "--thruput" ] || [ "$1" == "-t" ]
    then
        ncat --recv-only $2 10000 | pv > /dev/null
    fi
elif [ $# == 3 ]
then
    if [ "$1" == "--ncaplay" ] || [ "$1" == "-n" ]
    then
        ncat --recv-only $2 54321 | aplay --format=S32_LE --rate=$3 --file-type=raw --buffer-size=14000
    elif [ "$1" == "--playto" ] || [ "$1" == "-p" ]
    then
        cat $3 | ncat $2 12345
    fi
else
    echo "Error! --help or -h for help"
fi

