import SwiftUI

struct ContentView: View {
    private let greeting = EchoesGreeting.greeting()
    
    var body: some View {
        HStack {
            Image(systemName: "waveform")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text(greeting)
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
