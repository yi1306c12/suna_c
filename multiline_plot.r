csvname1 = commandArgs(trailingOnly=TRUE)[1]
csvname2 = commandArgs(trailingOnly=TRUE)[2]


png(paste("multiline",".png",sep=""))

table1 <- read.csv(csvname1)
ave1 <- apply(table1,1,mean)
table2 <- read.csv(csvname2)
ave2 <- apply(table2,1,mean)
ave <- cbind(ave1,ave2)/5
print(ave)
matplot(ave,type="l",ylim=c(0,50),xlab="Number of Generations",ylab="Accumerated Reward")
legend("topleft",legend=c("original input","reduced input"),lty=c(1,1),col=c(1,2))
dev.off()
