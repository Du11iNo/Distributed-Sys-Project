package client;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

// Imports required for RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;

// Imports required for RMI
import java.io.IOException;
import java.rmi.Naming;
import java.io.File;

public class Client {
    private final static String QUEUE_NAME = "JR"; // TODO (change queue name)

    static int id; // Client identifier
    static int maxChunkSize; // Maximum size of the chunks to download

    public static boolean directoryExists(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }

        File file = new File(path);
        return file.exists() && file.isDirectory();
    }

    public static boolean fileExists(String path) {
        if (path == null || path.isEmpty()) {
            return false;
        }

        File file = new File(path);
        return file.exists() && file.isFile();
    }

    public static void main(String[] argv) throws Exception {

        // =================================================
        // Get command line arguments
        if (argv.length < 4) {
            System.out.println("Usage: client <client_id> <requests_file> <downloads_directory> <max_chunk_size>");
            System.exit(1);
        }

        try {
            id = Integer.parseInt(argv[0]);
        } catch (NumberFormatException e) {
            System.out.println("The client id must be an integer");
            System.exit(2);
        }

        if (!fileExists(argv[1])) {
            System.out.println("Client " + id + ": The requests file does not exist");
            System.exit(3);
        }

        // Check if the downloads directory exists
        if (!directoryExists(argv[2])) {
            System.out.println("Client " + id + ": The downloads directory does not exist");
            System.exit(4);
        }

        try {
            maxChunkSize = Integer.parseInt(argv[3]);
        } catch (NumberFormatException e) {
            System.out.println("The maximum chunk size must be an integer");
            System.exit(5);
        }

        if (maxChunkSize <= 0) {
            System.out.println("The maximum chunk size must be greater than 0");
            System.exit(6);
        }

        // =================================================
        // Main part, all within a try block to catch any exception
        try {

            // Start the RMI server part of the client and register it
            // with rmiregistry using the appropriate name for this client
            // TODO:
            ClientImpl cli = new ClientImpl();
            Naming.rebind("Client_" + id, cli);

            // Connect to RabbitMQ to send requests to the queue
            // TODO:
            ConnectionFactory factory = new ConnectionFactory();
            factory.setHost("localhost");
            Connection connection = factory.newConnection();
            Channel channel = connection.createChannel();

            channel.queueDeclare(QUEUE_NAME, false, false, false, null);

            // Process the file downloads
            processDownloads(channel, cli, argv[1], argv[2]);

            // Finish
            System.out.println("Client finished");
            channel.close();
            connection.close();
            System.exit(0);
        } catch (Exception e) {
            // Any exception is simply printed
            System.out.println("Error in Client: " + e.getMessage());
            e.printStackTrace();
        }
    }

    static void downloadFile(Channel channel, ClientImpl cli, String filename, String download_directory) {
        // This function is not used in this client, but it is left here as an example
        // of how a file download could be implemented.
        // FileOutputStream could be used to write the file into the specified
        // directory.
        String msg; // To compose the messages to be sent via RabbitMQ
        Long file_size;

        System.out.println("Downloading file " + filename + " to " + download_directory);

        // The request first consists of the client name registered in RMI
        // followed by the command and the file name, both separated by
        // white spaces. This message is sent to the RabbitMQ queue.
        try {
            // TODO
            msg = "Client_" + String.valueOf(id) + " GETSIZE " + filename;

            System.out.println(msg);

            channel.basicPublish("", QUEUE_NAME, null, msg.getBytes("UTF-8"));
            file_size = cli.getSize();

            try {
                FileOutputStream stream = new FileOutputStream(download_directory + filename);
                Long offset = Long.valueOf(0);

                while (offset < file_size) {
                    msg = "Client_" + String.valueOf(id) + " GETCHUNK " + filename + " " + String.valueOf(offset) + " "
                            + String.valueOf(maxChunkSize);
                    channel.basicPublish("", QUEUE_NAME, null, msg.getBytes("UTF-8"));
                    stream.write(cli.getChunk());
                    offset += maxChunkSize;
                }

                processTags(channel, cli, filename, download_directory);

            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        } catch (Exception e) {
            System.out.println("Client_" + id + ": Error downloading file " + filename);
            e.printStackTrace();
        }
    }

    static void processTags(Channel channel, ClientImpl cli, String filename, String download_directory) {
        // This function is not used in this client, but it is left here as an example
        // of how a file download could be implemented.
        // FileOutputStream could be used to write the file into the specified
        // directory.
        System.out.println("Processing tags of file " + filename + " in " + download_directory);
        // Logic for processing file tags would go here

        // Opens the file in the downloads directory
        String filePath = download_directory + "/" + filename;
        // We read the file content
        // until we find a "<" character indicating the start of a tag.
        // If we find said character, we check if it is followed by the
        // characters forming the string "img" or "IMG".
        try {
            BufferedReader br = new BufferedReader(new FileReader(filePath));
            String line;
            int pos; // Position of the <img or <IMG tag in the line
            while ((line = br.readLine()) != null) {
                pos = line.indexOf("<img");
                if (pos == -1) {
                    pos = line.indexOf("<IMG");
                }
                if (pos != -1) {
                    pos += 5; // Advance position to skip <img or <IMG and the white space
                    StringBuilder aux = new StringBuilder();
                    String image_name;

                    while (pos < line.length() && line.charAt(pos) != '>') {
                        aux.append(line.charAt(pos));
                        pos++;
                    }
                    image_name = aux.toString().trim(); // Get the image name
                    // TODO
                    try {
                        URI image = new URI(image_name);

                        InputStream in = image.toURL().openStream();
                        image_name = image_name.substring(image_name.lastIndexOf("/") + 1);
                        FileOutputStream out = new FileOutputStream(image_name);

                        byte[] buffer = new byte[4096];
                        int read = 0;

                        while ((read = in.read(buffer)) != -1) {
                            out.write(buffer, 0, read);
                        }
                        break;
                    } catch (URISyntaxException e) {
                        e.printStackTrace();
                    }
                }
            }
            br.close();
        } catch (IOException e) {
            System.out.println("Error reading file " + filename);
            e.printStackTrace();
        }
    }

    // =========================================================================
    // The processDownloads function reads from the requests file and sends
    // messages through the message queue to check if the file to download
    // exists on the server, and if it exists, obtain its size,
    // and proceed to download it in chunks of maximum size maxChunkSize.
    //
    // Requires as parameters:
    //
    // - The RabbitMQ communication channel to send messages
    // - The reference to the RMI object exported by the client
    // - The name of the file containing the queries to be sent
    // - The name of the folder where the downloaded files will be stored
    //
    // Once it finishes downloading all files from the input file,
    // the function returns.
    static void processDownloads(Channel channel, ClientImpl cli, String requests_file, String download_directory)
            throws IOException, InterruptedException {

        BufferedReader br = new BufferedReader(new FileReader(requests_file));
        String msg; // To compose the messages to be sent via RabbitMQ
        Long file_size;

        try {
            String query = br.readLine();

            while (query != null) {
                // TODO

                msg = "Client_" + String.valueOf(id) + " EXISTFILE " + query;

                channel.basicPublish("", QUEUE_NAME, null, msg.getBytes("UTF-8"));

                if (!cli.getExistFile()) {
                    query = br.readLine();
                    continue;
                }

                downloadFile(channel, cli, query, download_directory);

                // Read the next download request from the file
                query = br.readLine();
            }
            return;
        } catch (Exception e) {
            // Any exception is simply printed
            System.out.println("Error in Client: " + e.getMessage());
            e.printStackTrace();
        } finally {
            // Close input and output files
            br.close();
        }
    }
}
