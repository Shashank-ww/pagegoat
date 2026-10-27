
import type { Metadata } from "next";
import { Inter } from "next/font/google";
import SiteFooter from "./components/SiteFooter";
import "./globals.css";

const inter = Inter({
  subsets: ["latin"],
  variable: "--font-inter",
  display: "swap",
});

export const metadata: Metadata = {
  title: "Welookit - Company Discovery",
  description:
    "Find company websites, public contact details, social profiles, technology and website information.",
  applicationName: "Welookit",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body className={inter.variable}>
        {children}
        <SiteFooter />
      </body>
    </html>
  );
}
