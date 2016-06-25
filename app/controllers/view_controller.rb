class ViewController < ApplicationController
  def draw_chart
      @client = Mysql2::Client.new(:host => "128.199.114.194", :username => "hackathon", :password => "hackathon@like" ,:database => "hackathon")

      @rows = @client.query("SELECT HOUR(date), SUM(count), zone FROM user_cnt where zone = '#{params[:id]}' GROUP BY HOUR(date);")
      @result = Array.new
      @categories = Array.new
      @rows.each{ |row| @categories.push (row["HOUR(date)"] )}
      @rows.each { |row| @result.push (row["SUM(count)"])}


  end
end
